#include "DirectionalLight.h"
#include "GameFramework/Object/Component/DirectionalLightComponent.h"

ADirectionalLight::ADirectionalLight()
{
	mDirectionalLightComponent = CreateSceneComponent<WDirectionalLightComponent>();
	SetRootComponent(mDirectionalLightComponent);
}