﻿#include "stdafx.h"
#include "gmdx11shaderprogram.h"
#include <linearmath.h>

namespace
{
	inline const GMShaderVariablesDesc& GMGetDefaultShaderVariablesDesc()
	{
		static GMShaderVariablesDesc desc =
		{
			"GM_WorldMatrix",
			"GM_ViewMatrix",
			"GM_ProjectionMatrix",
			"GM_InverseTransposeModelMatrix",
			"GM_InverseViewMatrix",

			"GM_ViewPosition",

			{ "OffsetX", "OffsetY", "ScaleX", "ScaleY", "Enabled" },
			"GM_AmbientTextureAttribute",
			"GM_DiffuseTextureAttribute",
			"GM_SpecularTextureAttribute",
			"GM_NormalMapTextureAttribute",
			"GM_LightmapTextureAttribute",
			"GM_AlbedoTextureAttribute",
			"GM_MetallicRoughnessAOTextureAttribute",
			"GM_CubeMapTextureAttribute",

			"GM_LightCount",

			{ "Ka", "Kd", "Ks", "Shininess", "Refractivity", "F0" },
			"GM_Material",

			{
				"GM_Filter",
				"GM_KernelDeltaX",
				"GM_KernelDeltaY",
				{
					"GM_DefaultFilter",
					"GM_InversionFilter",
					"GM_SharpenFilter",
					"GM_BlurFilter",
					"GM_GrayscaleFilter",
					"GM_EdgeDetectFilter",
				}
			},

			{
				"GM_ScreenInfo",
				"ScreenWidth",
				"ScreenHeight",
				"Multisampling",
			},

			"GM_RasterizerState",
			"GM_BlendState",
			"GM_DepthStencilState",

			{
				"GM_ShadowInfo",
				"HasShadow",
				"ShadowMatrix",
				"Position",
				"GM_ShadowMap",
				"GM_ShadowMapMSAA",
				"ShadowMapWidth",
				"ShadowMapHeight",
				"BiasMin",
				"BiasMax",
			},

			{
				"GM_GammaCorrection",
				"GM_Gamma",
				"GM_GammaInv",
			},

			{
				"GM_HDR",
				"GM_ToneMapping",
			},

			"GM_IlluminationModel",
		};
		return desc;
	}
}

GMDx11EffectShaderProgram::GMDx11EffectShaderProgram(GMComPtr<ID3DX11Effect> effect)
{
	D(d);
	d->effect = effect;
}

void GMDx11EffectShaderProgram::useProgram()
{
}

void GMDx11EffectShaderProgram::setMatrix4(const char* name, const GMMat4& value)
{
	D(d);
	ID3DX11EffectMatrixVariable* var = getMatrixVariable(name);
	GM_DX_HR(var->SetMatrix(ValuePointer(value)));
}

void GMDx11EffectShaderProgram::setVec4(const char* name, const GMFloat4& vector)
{
	D(d);
	ID3DX11EffectVectorVariable* var = getVectorVariable(name);
	GM_DX_HR(var->SetFloatVector(ValuePointer(vector)));
}

void GMDx11EffectShaderProgram::setVec3(const char* name, const GMfloat value[3])
{
	D(d);
	GM_ASSERT(false); //not supported
}

void GMDx11EffectShaderProgram::setInt(const char* name, GMint value)
{
	D(d);
	ID3DX11EffectScalarVariable* var = getScalarVariable(name);
	GM_DX_HR(var->SetInt(value));
}

void GMDx11EffectShaderProgram::setFloat(const char* name, GMfloat value)
{
	D(d);
	ID3DX11EffectScalarVariable* var = getScalarVariable(name);
	GM_DX_HR(var->SetFloat(value));
}

void GMDx11EffectShaderProgram::setBool(const char* name, bool value)
{
	D(d);
	ID3DX11EffectScalarVariable* var = getScalarVariable(name);
	GM_DX_HR(var->SetBool(value));
}

bool GMDx11EffectShaderProgram::setInterfaceInstance(const char* interfaceName, const char* instanceName, GMShaderType type)
{
	D(d);
	ID3DX11EffectInterfaceVariable* interfaceVariable = getInterfaceVariable(interfaceName);
	ID3DX11EffectClassInstanceVariable* instanceVariable = getInstanceVariable(instanceName);
	if (instanceVariable->IsValid())
	{
		GM_DX_HR_RET(interfaceVariable->SetClassInstance(instanceVariable));
	}
	else
	{
		return false;
	}
	return true;
}

bool GMDx11EffectShaderProgram::setInterface(GameMachineInterfaceID id, void* in)
{
	return false;
}

bool GMDx11EffectShaderProgram::getInterface(GameMachineInterfaceID id, void** out)
{
	D(d);
	if (id == GameMachineInterfaceID::D3D11Effect)
	{
		d->effect->AddRef();
		(*out) = d->effect;
		return true;
	}
	return false;
}

const GMShaderVariablesDesc& GMDx11EffectShaderProgram::getDesc()
{
	return GMGetDefaultShaderVariablesDesc();
}

ID3DX11EffectVectorVariable* GMDx11EffectShaderProgram::getVectorVariable(const char* name)
{
	D(d);
	auto& container = d->vectors;
	decltype(container.find("")) iter = container.find(name);
	if (iter == container.end())
	{
		ID3DX11EffectVectorVariable* var = d->effect->GetVariableByName(name)->AsVector();
		container[name] = var;
		return var;
	}
	return iter->second;
}

ID3DX11EffectMatrixVariable* GMDx11EffectShaderProgram::getMatrixVariable(const char* name)
{
	D(d);
	auto& container = d->matrices;
	decltype(container.find("")) iter = container.find(name);
	if (iter == container.end())
	{
		ID3DX11EffectMatrixVariable* var = d->effect->GetVariableByName(name)->AsMatrix();
		container[name] = var;
		return var;
	}
	return iter->second;
}

ID3DX11EffectScalarVariable* GMDx11EffectShaderProgram::getScalarVariable(const char* name)
{
	D(d);
	auto& container = d->scalars;
	decltype(container.find("")) iter = container.find(name);
	if (iter == container.end())
	{
		ID3DX11EffectScalarVariable* var = d->effect->GetVariableByName(name)->AsScalar();
		container[name] = var;
		return var;
	}
	return iter->second;
}

ID3DX11EffectInterfaceVariable* GMDx11EffectShaderProgram::getInterfaceVariable(const char* name)
{
	D(d);
	auto& container = d->interfaces;
	decltype(container.find("")) iter = container.find(name);
	if (iter == container.end())
	{
		ID3DX11EffectInterfaceVariable* var = d->effect->GetVariableByName(name)->AsInterface();
		container[name] = var;
		return var;
	}
	return iter->second;
}

ID3DX11EffectClassInstanceVariable* GMDx11EffectShaderProgram::getInstanceVariable(const char* name)
{
	D(d);
	auto& container = d->instances;
	decltype(container.find("")) iter = container.find(name);
	if (iter == container.end())
	{
		ID3DX11EffectClassInstanceVariable* var = d->effect->GetVariableByName(name)->AsClassInstance();
		container[name] = var;
		return var;
	}
	return iter->second;
}