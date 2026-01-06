#include "DirectionalLight.h"
#include "GameFramework/Object/Component/DirectionalLightComponent.h"

ADirectionalLight::ADirectionalLight()
{
	mDirectionalLightComponent = CreateComponent<WDirectionalLightComponent>();
	SetRootComponent(mDirectionalLightComponent);
}