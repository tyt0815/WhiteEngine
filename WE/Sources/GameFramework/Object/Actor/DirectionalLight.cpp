#include "DirectionalLight.h"
#include "GameFramework/Object/Component/DirectionalLightComponent.h"

ADirectionalLight::ADirectionalLight()
{
	DirectionalLightComponent = CreateSceneComponent<WDirectionalLightComponent>();
	SetRootComponent(DirectionalLightComponent);
}