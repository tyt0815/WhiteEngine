#pragma once
#include <memory>
#include "LightComponent.h"
#include "Render/DepthStencil.h"

class WDirectionalLightComponent : public WLightComponent
{
	typedef WLightComponent Super;
public:
	WDirectionalLightComponent();

protected:
	virtual void Update() override;
	
	const size_t mDirectionalLightInfoPoolIndex = -1;
	std::unique_ptr<FDepthStencil> mShadowMap;
private:
};