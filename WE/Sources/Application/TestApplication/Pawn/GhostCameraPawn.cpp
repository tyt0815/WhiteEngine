#include "GhostCameraPawn.h"
#include "GameFramework/Object/World/World.h"
#include "GameFramework/Object/Component/CameraComponent.h"

AGhostCameraPawn::AGhostCameraPawn()
{
	Camera = CreateSceneComponent<WCameraComponent>();
	SetRootComponent(Camera);
}
