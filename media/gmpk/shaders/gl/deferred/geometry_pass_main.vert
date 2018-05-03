#version 410
#include "../foundation/foundation.h"
#include "../foundation/vert_header.h"

out vec4 _deferred_geometry_pass_position_world;

void deferred_geometry_pass_calcCoords()
{
	gl_Position = GM_projection_matrix * GM_view_matrix * GM_model_matrix * position;
	_deferred_geometry_pass_position_world = GM_model_matrix * position;
	_normal = normal;
	_tangent = tangent;
	_bitangent = bitangent;
	_uv = uv;
	_lightmapuv = lightmapuv;
}

subroutine (GM_TechniqueEntrance)
void GM_GeometryPass()
{
	deferred_geometry_pass_calcCoords();
}

void main(void)
{
	init_layouts();
	//GM_techniqueEntrance();
	GM_GeometryPass();
}