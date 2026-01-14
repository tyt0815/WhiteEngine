#include "DefaultWorld.h"
#include "../Actor/DirectionalLight.h"
#include "../Actor/Floor.h"
#include "../Component/DirectionalLightComponent.h"
#include "../Pawn/GhostCameraPawn.h"

WDefaultWorld::WDefaultWorld()
{
	APawn* Player = SpawnActor<AGhostCameraPawn>();
	SetPlayer(Player);

	ADirectionalLight* LightActor;
	LightActor = SpawnActor<ADirectionalLight>();
	LightActor->SetActorRotation(XMFLOAT3(0.0f, -45, -45));
	LightActor->GetDirLightComp()->SetColor({ 10.0f, 10.0f, 10.0f });

	AFloor* Floor = SpawnActor<AFloor>();
	Floor->SetActorLocation(XMFLOAT3(0.0f, -5.0f, 0.0f));
}
