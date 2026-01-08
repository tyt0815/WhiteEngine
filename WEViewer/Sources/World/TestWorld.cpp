#include "TestWorld.h"
#include "../Actor/Box.h"
#include "../Actor/Floor.h"
#include "../Actor/Sphere.h"
#include "../Actor/ProjectileSphere.h"
#include "../Pawn/GhostCameraPawn.h"
#include "GameFramework/Object/Actor/DirectionalLight.h"
#include "GameFramework/Object/Component/DirectionalLightComponent.h"
#include "../Actor/ProjectileSpawner.h"

WTestWorld::WTestWorld()
{
	APawn* Player = SpawnActor<AGhostCameraPawn>();
	SetPlayer(Player);

	AActor* Actor = nullptr;
	FTransform Transform = FTransform::Default;

	ADirectionalLight* LightActor;
	LightActor = SpawnActor<ADirectionalLight>();
	Transform.Rotation = XMFLOAT3(0.0f, -45, -45);
	LightActor->SetActorTransform(Transform);
	LightActor->GetDirLightComp()->SetColor({ 10.0f, 10.0f, 10.0f });

	//Actor = SpawnActor<ASphere>();
	//Transform.Scale = XMFLOAT3(2.0f, 2.0f, 2.0f);
	//Transform.Translation = XMFLOAT3(2.0f, 0.0f, 5.0f);
	//Actor->SetActorTransform(Transform);

	//Actor = SpawnActor<ABox>();
	//Transform.Translation = XMFLOAT3(-2.0f, 0.0f, 5.0f);
	//Actor->SetActorTransform(Transform);

	//Actor = SpawnActor<AFloor>();
	//Transform.Translation = XMFLOAT3(0.0f, -2.0f, 0.0f);
	//Actor->SetActorTransform(Transform);

	Actor = SpawnActor<AProjectileSpawner>();
	Transform.Translation = XMFLOAT3(0.0f, 0.0f, 5.0f);
	Transform.Rotation = XMFLOAT3(0.0f, 90.0f, 0.0f);
	Actor->SetActorTransform(Transform);
}
