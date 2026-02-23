#include "DefaultWorld.h"
#include "../Actor/DirectionalLight.h"
#include "../Actor/Floor.h"
#include "../Component/DirectionalLightComponent.h"
#include "../Pawn/GhostCameraPawn.h"
#include "GUI/GUICore.h"

void WDefaultWorld::BeginPlay()
{
	Super::BeginPlay();

	if (GetPlayer().expired())
	{
		auto Player = SpawnActor<AGhostCameraPawn>();
		SetPlayer(Player->GetWeakPtr<APawn>());
	}

	auto LightActor = SpawnActor<ADirectionalLight>();
	LightActor->SetActorRotation(XMFLOAT3(0.0f, -45, -45));
	LightActor->GetDirLightComp().lock()->SetColor({ 10.0f, 10.0f, 10.0f });

	FActorSpawnParameter Param;
	Param.Transform.Translation = XMFLOAT3(0.0f, -2.0f, 0.0f);
	auto Floor = SpawnActor<AFloor>(Param);
}
