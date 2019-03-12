﻿#include "stdafx.h"
#include "handler_control.h"
#include <gmutilities.h>
#include "handler.h"
#include <gmlight.h>

namespace
{
	void setLightAttributes(ILight* light, const GMVec3& position, const GMVec3& diffuseIntensity, const GMVec3& ambientIntensity)
	{
		GM_ASSERT(light);
		light->setLightAttribute3(GMLight::Position, ValuePointer(position));
		light->setLightAttribute3(GMLight::DiffuseIntensity, ValuePointer(diffuseIntensity));
		light->setLightAttribute3(GMLight::AmbientIntensity, ValuePointer(ambientIntensity));
	}

	GMCamera defaultCamera()
	{
		static std::once_flag s_flag;
		static GMCamera s_camera;
		std::call_once(s_flag, [](GMCamera&){
			s_camera.setOrtho(-1, 1, -1, 1, .1f, 3200.f);
			gm::GMCameraLookAt lookAt;
			lookAt.lookAt = { 0, 0, 1 };
			lookAt.position = { 0, 0, -1 };
			s_camera.lookAt(lookAt);
		}, s_camera);
		return s_camera;
	}
}

namespace core
{
	HandlerControl::HandlerControl(Handler* handler, QObject* parent /*= nullptr*/)
		: QObject(parent)
		, m_handler(handler)
	{
		setDefaultLight(GMVec3(0, 0, -.2f), GMVec3(.7f, .7f, .7f), GMVec3(0, 0, 0));
		setDefaultCamera(defaultCamera());
	}

	void HandlerControl::setDefaultCamera(const GMCamera& camera)
	{
		auto engine = m_handler->getContext()->getEngine();
		engine->setCamera(camera);
	}

	void HandlerControl::setDefaultColor(const GMVec4& color)
	{
		auto engine = m_handler->getContext()->getEngine();
		engine->getDefaultFramebuffers()->setClearColor(ValuePointer(color));
	}

	void HandlerControl::setDefaultLight(const GMVec3& position, const GMVec3& diffuseIntensity, const GMVec3& ambientIntensity)
	{
		auto engine = m_handler->getContext()->getEngine();
		if (m_lights.defaultLightIndex == -1)
		{
			ILight* light = nullptr;
			GM.getFactory()->createLight(GMLightType::PointLight, &light);
			setLightAttributes(light, position, diffuseIntensity, ambientIntensity);
			m_lights.defaultLightIndex = engine->addLight(light);
		}
		else
		{
			ILight* light = engine->getLight(m_lights.defaultLightIndex);
			setLightAttributes(light, position, diffuseIntensity, ambientIntensity);
			engine->update(GMUpdateDataType::LightChanged);
		}
	}

	void HandlerControl::clearRenderList()
	{
		m_handler->getWorld()->clearRenderList();
	}

	void HandlerControl::renderLogo()
	{
		if (m_assets.logo.isEmpty())
		{
			m_assets.logo = createLogo();
			m_assets.logoObj = new GMGameObject(m_assets.logo);
			m_handler->getWorld()->addObjectAndInit(m_assets.logoObj);
		}
		m_handler->getWorld()->addToRenderList(m_assets.logoObj);
	}

	GMSceneAsset HandlerControl::createLogo()
	{
		// 创建一个带纹理的对象
		GMVec2 extents = GMVec2(1.f, .5f);
		GMSceneAsset asset;
		GMPrimitiveCreator::createQuadrangle(extents, 0, asset);

		GMModel* model = asset.getScene()->getModels()[0].getModel();
		model->getShader().getMaterial().setDiffuse(GMVec3(1, 1, 1));
		model->getShader().getMaterial().setSpecular(GMVec3(0));

		GMTextureAsset tex = GMToolUtil::createTexture(m_handler->getContext(), "gamemachine.png"); //TODO 考虑从qrc拿
		GMToolUtil::addTextureToShader(model->getShader(), tex, GMTextureType::Diffuse);
		return asset;
	}
}