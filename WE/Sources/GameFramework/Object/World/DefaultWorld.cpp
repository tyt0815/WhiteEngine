#include "DefaultWorld.h"
#include "../Actor/DirectionalLight.h"
#include "../Actor/Floor.h"
#include "../Component/DirectionalLightComponent.h"
#include "../Pawn/GhostCameraPawn.h"
#include "GUI/GUICore.h"

WDefaultWorld::WDefaultWorld()
{
	auto Player = SpawnActor<AGhostCameraPawn>();
	SetPlayer(Player);

	auto LightActor = SpawnActor<ADirectionalLight>().lock();
	LightActor->SetActorRotation(XMFLOAT3(0.0f, -45, -45));
	LightActor->GetDirLightComp().lock()->SetColor({ 10.0f, 10.0f, 10.0f });

	auto Floor = SpawnActor<AFloor>().lock();
	Floor->SetActorLocation(XMFLOAT3(0.0f, -5.0f, 0.0f));
}

void WDefaultWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
