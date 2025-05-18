#pragma once
#include "LightComponent.h"

class WDirectionalLightComponent : public WLightComponent
{
	typedef WLightComponent Super;
public:
	WDirectionalLightComponent();

protected:
	virtual void Update() override;
	
	const size_t mDirectionalLightInfoPoolIndex;
private:
};