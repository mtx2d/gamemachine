﻿#ifndef __GMGLLIGHT_H__
#define __GMGLLIGHT_H__
#include "common.h"
#include "gmengine/controller/light.h"
BEGIN_NS

class GMGLShaders;
class GMGLLight : public LightController
{
public:
	GMGLLight(GMGLShaders& shaders);

public:
	virtual void setAmbient(GMfloat rgb[3]) override;
	virtual void setAmbientCoefficient(GMfloat ka[3]) override;
	virtual void setLightColor(GMfloat rgb[3]) override;
	virtual void setDiffuseCoefficient(GMfloat kd[3]) override;
	virtual void setShininess(GMfloat n) override;
	virtual void setSpecularCoefficient(GMfloat ks[3]) override;
	virtual void setLightPosition(GMfloat xyz[3]) override;
	virtual void setViewPosition(GMfloat xyz[3]) override;

private:
	GMGLShaders& m_shaders;
};

END_NS
#endif