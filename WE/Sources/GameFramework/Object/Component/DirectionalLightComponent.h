#pragma once
#include "LightComponent.h"

class WDirectionalLightComponent : public WLightComponent
{
	typedef WLightComponent Super;
protected:
	virtual void Update() override;
};