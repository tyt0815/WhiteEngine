#include "TestWorld.h"
#include "Render/MeshGeometry.h"
#include "GameFramework/Object/Actor/ShapeActorsHeader.h"

WTestWorld::WTestWorld():
	Super()
{
}

void WTestWorld::BuildWorldActors()
{
	AActor* Actor = nullptr;
	FTransform Transform;
	Transform.Scale = XMFLOAT3(2.0f, 2.0f, 2.0f);
	Transform.Translation = XMFLOAT3(0.0f, 0.5f, 0.0f);
	Actor = SpawnActor<AWireFence>();
	Actor->SetTransform(Transform);

	Transform = FTransform::Default;
	Actor = SpawnActor<AGrid>();
	Actor->SetTransform(Transform);

	Transform = FTransform::Default;
	Transform.Scale = XMFLOAT3(0.5f, 0.5f, 0.5f);
	Transform.Translation = XMFLOAT3(0.0f, 1.0f, 0.0f);
	Actor = SpawnActor<ASkull>();
	Actor->SetTransform(Transform);


	for (int i = 0; i < 50; ++i)
	{
		// LeftCylinder
		Transform = FTransform::Default;
		Transform.Translation = XMFLOAT3(+5.0f, 1.5f, -10.0f + i * 5.0f);
		Actor = SpawnActor<ACylinder>();
		Actor->SetTransform(Transform);
		// RightCylinder
		Transform = FTransform::Default;
		Transform.Translation = XMFLOAT3(-5.0f, 1.5f, -10.0f + i * 5.0f);
		Actor = SpawnActor<ACylinder>();
		Actor->SetTransform(Transform);
		// LeftSphere
		Transform = FTransform::Default;
		Transform.Translation = XMFLOAT3(-5.0f, 3.5f, -10.0f + i * 5.0f);
		Actor = SpawnActor<AWaterBall>();
		Actor->SetTransform(Transform);
		// RightSphere
		Transform = FTransform::Default;
		Transform.Translation = XMFLOAT3(+5.0f, 3.5f, -10.0f + i * 5.0f);
		Actor = SpawnActor<AWaterBall>();
		Actor->SetTransform(Transform);
	}
}
