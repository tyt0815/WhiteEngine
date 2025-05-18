#include "TestWorld.h"
#include "Render/MeshGeometry.h"
#include "Application/TestApplication/Pawn/GhostCameraPawn.h"
#include "Application/TestApplication/Actor/IceFieldGird.h"
#include "Application/TestApplication/Actor/RustedIron2Sphere.h"
#include "Application/TestApplication/Actor/ScuffedGoldBox.h"
#include "Application/TestApplication/Actor/ScuffedGoldSphere.h"
#include "Application/TestApplication/Actor/ThickMortarStonework.h"
#include "Application/TestApplication/Actor/LaminateFloorBrown.h"
#include "Application/TestApplication/Actor/RotatingDirLight.h"

WTestWorld::WTestWorld()
{
	APawn* Player = SpawnActor<AGhostCameraPawn>();
	SetPlayer(Player);
	

	AActor* Actor = nullptr;
	FTransform Transform = FTransform::Default;

	Actor = SpawnActor<ARustedIron2Sphere>();
	Transform.Scale = XMFLOAT3(2.0f, 2.0f, 2.0f);
	Transform.Translation = XMFLOAT3(0.0f, 0.0f, 5.0f);
	Actor->SetActorTransform(Transform);

	Actor = SpawnActor<AScuffedGoldSphere>();
	Transform.Translation = XMFLOAT3(2.0f, 0.0f, 5.0f);
	Actor->SetActorTransform(Transform);

	Actor = SpawnActor<AThickMortarStonework>();
	Transform.Translation = XMFLOAT3(-2.0f, 0.0f, 5.0f);
	Actor->SetActorTransform(Transform);

	Actor = SpawnActor<AScuffedGoldBox>();
	Transform.Translation = XMFLOAT3(4.0f, 0.0f, 5.0f);
	Actor->SetActorTransform(Transform);

	Actor = SpawnActor<ALaminateFloorBrown>();
	Transform.Translation = XMFLOAT3(0.0f, -2.0f, 0.0f);
	Actor->SetActorTransform(Transform);

	Actor = SpawnActor<ARotatingDirLight>();
}
