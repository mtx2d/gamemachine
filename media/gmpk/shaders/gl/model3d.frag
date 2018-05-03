// 相机视角法向量
vec3 g_model3d_normal_eye;
vec3 g_model3d_ambientLight;
vec3 g_model3d_diffuseLight;
vec3 g_model3d_specularLight;
vec3 g_model3d_refractionLight;

in vec4 _model3d_position_world;

vec3 saturate(vec3 vector)
{
	return clamp(vector, 0.0f, 1.0f);
}

void model3d_init()
{
	g_model3d_ambientLight = g_model3d_diffuseLight = g_model3d_specularLight = vec3(0);
}

void model3d_calculateRefractionByNormalWorld(vec3 normal_world)
{
	if (GM_material.refractivity == 0.f)
		return;

	vec3 I = normalize((_model3d_position_world - GM_view_position).rgb);
	vec3 R = refract(I, normal_world, GM_material.refractivity);
	g_model3d_refractionLight += texture(GM_cubemap_texture, vec3(R.x, R.y, R.z)).rgb;
}

void model3d_calculateRefractionByNormalTangent(mat3 TBN, vec3 normal_tangent)
{
	if (GM_material.refractivity == 0.f)
		return;
	
	// 如果是切线空间，计算会复杂点，要将切线空间的法线换算回世界空间
	vec3 normal_world = (GM_inverse_view_matrix * vec4(transpose(TBN) * normal_tangent, 0)).rgb;
	model3d_calculateRefractionByNormalWorld(normal_world);
}

void model3d_calcLights()
{
	// 由顶点变换矩阵计算法向量变换矩阵
	mat3 inverse_transpose_model_matrix = mat3(GM_inverse_transpose_model_matrix);
	mat3 normalEyeTransform = mat3(GM_view_matrix) * inverse_transpose_model_matrix;
	vec4 vertex_eye = GM_view_matrix * _model3d_position_world;
	vec3 eyeDirection_eye = vec3(0,0,0) - vertex_eye.xyz;
	vec3 eyeDirection_eye_N = normalize(eyeDirection_eye);
	// normal的齐次向量最后一位必须位0，因为法线变换不考虑平移
	g_model3d_normal_eye = normalize( (normalEyeTransform * _normal.xyz) );

	// 计算漫反射和高光部分
	if (GM_normalmap_textures[0].enabled == 0)
	{
		for (int i = 0; i < GM_lightCount; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_lights[i].lightPosition, 1)).xyz;
			vec3 lightDirection_eye_N = normalize(lightPosition_eye + eyeDirection_eye);
			g_model3d_ambientLight += GM_material.ka * GMLight_Ambient(GM_lights[i]);
			g_model3d_diffuseLight += GM_material.kd * GMLight_Diffuse(GM_lights[i], lightDirection_eye_N, eyeDirection_eye_N, g_model3d_normal_eye);
			g_model3d_specularLight += GM_material.ks * GMLight_Specular(GM_lights[i], lightDirection_eye_N, eyeDirection_eye_N, g_model3d_normal_eye, GM_material.shininess);
			if (GM_lights[i].lightType == GM_AmbientLight)
				model3d_calculateRefractionByNormalWorld(normalize(inverse_transpose_model_matrix * _normal.xyz).xyz);
		}
	}
	else
	{
		vec3 normal_tangent = texture(GM_normalmap_textures[0].texture, _uv).rgb * 2.0 - 1.0;
		vec3 tangent_eye = normalize((normalEyeTransform * _tangent.xyz).xyz);
		vec3 bitangent_eye = normalize((normalEyeTransform * _bitangent.xyz).xyz);
		for (int i = 0; i < GM_lightCount; i++)
		{
			vec3 lightPosition_eye = (GM_view_matrix * vec4(GM_lights[i].lightPosition, 1)).xyz;
			vec3 lightDirection_eye = lightPosition_eye + eyeDirection_eye;
			// TBN变换矩阵
			mat3 TBN = transpose(mat3(
				tangent_eye,
				bitangent_eye,
				g_model3d_normal_eye.xyz
			));
			vec3 lightDirection_tangent_N = normalize(TBN * lightDirection_eye);
			vec3 eyeDirection_tangent_N = normalize(TBN * eyeDirection_eye);

			g_model3d_ambientLight += GM_material.ka * GMLight_Ambient(GM_lights[i]);
			g_model3d_diffuseLight += GM_material.kd * GMLight_Diffuse(GM_lights[i], lightDirection_tangent_N, eyeDirection_tangent_N, normal_tangent);
			g_model3d_specularLight += GM_material.ks * GMLight_Specular(GM_lights[i], lightDirection_tangent_N, eyeDirection_tangent_N, normal_tangent, GM_material.shininess);
			if (GM_lights[i].lightType == GM_AmbientLight)
				model3d_calculateRefractionByNormalTangent(TBN, normal_tangent);
		}
	}
}

vec3 model3d_calcTexture(GM_texture_t textures[MAX_TEXTURE_COUNT], vec2 uv, int size)
{
	bool hasTexture = false;
	vec3 result = vec3(0);
	for (int i = 0; i < size; i++)
	{
		if (textures[i].enabled == 0)
			break;
		
		result += textures[i].enabled == 1
			? vec3(texture(textures[i].texture, uv * vec2(textures[i].scale_s, textures[i].scale_t) + vec2(textures[i].scroll_s, textures[i].scroll_t)))
			: vec3(0);
		if (textures[i].enabled == 1)
			hasTexture = true;
	}

	if (!hasTexture)
		return vec3(1);

	return result;
}

void model3d_calcColor()
{
	model3d_calcLights();

	if (GM_debug_draw_normal == 1)
	{
		// 画眼睛视角的法向量
		// 采用左手坐标系，法线z坐标应该反转
		g_model3d_normal_eye.z = -g_model3d_normal_eye.z;
		_frag_color = vec4((g_model3d_normal_eye.xyz + 1.f) / 2.f, 1.f);
		return;
	}
	else if (GM_debug_draw_normal == 2)
	{
		// 画世界视角的法向量
		// 采用左手坐标系，法线z坐标应该反转
		vec3 normal = vec3(_normal.x, _normal.y, -_normal.z);
		_frag_color = vec4((normal + 1.f) / 2.f, 1.f);
		return;
	}
	vec3 diffuseTextureColor = model3d_calcTexture(GM_diffuse_textures, _uv, MAX_TEXTURE_COUNT);
	vec3 ambientTextureColor = model3d_calcTexture(GM_ambient_textures, _uv, MAX_TEXTURE_COUNT) *
		model3d_calcTexture(GM_lightmap_textures, _lightmapuv, 1);

	// 最终结果
	vec3 color = saturate(g_model3d_ambientLight) * ambientTextureColor
	  + saturate(g_model3d_diffuseLight) * diffuseTextureColor
	  + saturate(g_model3d_specularLight) + saturate(g_model3d_refractionLight);
	_frag_color = vec4(color, 1.0f);
}

subroutine (GM_TechniqueEntrance)
void GM_Model3D()
{
	model3d_init();
	model3d_calcColor();
}
