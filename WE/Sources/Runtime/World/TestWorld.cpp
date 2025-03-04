#include "TestWorld.h"
#include "Render/MeshGeometry.h"
#include "Runtime/Object/ShapeActorsHeader.h"

WTestWorld::WTestWorld():
	Super()
{
}

void WTestWorld::BuildWorldActors()
{
	FTransform Transform;
	Transform.Scale = XMFLOAT3(2.0f, 2.0f, 2.0f);
	Transform.Translation = XMFLOAT3(0.0f, 0.5f, 0.0f);
	SpawnActor<AWireFence>(EActorType::EAT_AlphaTest, Transform);

	Transform = FTransform::Default;
	SpawnActor<AGrid>(EActorType::EAT_Opaque, Transform);

	Transform = FTransform::Default;
	Transform.Scale = XMFLOAT3(0.5f, 0.5f, 0.5f);
	Transform.Translation = XMFLOAT3(0.0f, 1.0f, 0.0f);
	SpawnActor<ASkull>(EActorType::EAT_Opaque, Transform);


	for (int i = 0; i < 50; ++i)
	{
		// LeftCylinder
		Transform = FTransform::Default;
		Transform.Translation = XMFLOAT3(+5.0f, 1.5f, -10.0f + i * 5.0f);
		SpawnActor<ACylinder>(EActorType::EAT_Opaque, Transform);
		// RightCylinder
		Transform = FTransform::Default;
		Transform.Translation = XMFLOAT3(-5.0f, 1.5f, -10.0f + i * 5.0f);
		SpawnActor<ACylinder>(EActorType::EAT_Opaque, Transform);
		// LeftSphere
		Transform = FTransform::Default;
		Transform.Translation = XMFLOAT3(-5.0f, 3.5f, -10.0f + i * 5.0f);
		SpawnActor<AWaterBall>(EActorType::EAT_Transparency, Transform);
		// RightSphere
		Transform = FTransform::Default;
		Transform.Translation = XMFLOAT3(+5.0f, 3.5f, -10.0f + i * 5.0f);
		SpawnActor<AWaterBall>(EActorType::EAT_Transparency, Transform);
	}

	/*for (int i = 0; i < 1000; ++i)
	{
		float Offset = 40;
		Transform = FTransform::Default;
		Transform.Translation.x = Offset * FDXMath::RandF();
		Transform.Translation.z = Offset * FDXMath::RandF();
		SpawnActor<AFoliage>(EActorType::EAT_Billboard, Transform);

		Transform.Translation.x = -Offset * FDXMath::RandF();
		Transform.Translation.z = Offset * FDXMath::RandF();
		SpawnActor<AFoliage>(EActorType::EAT_Billboard, Transform);

		Transform.Translation.x = Offset * FDXMath::RandF();
		Transform.Translation.z = -Offset * FDXMath::RandF();
		SpawnActor<AFoliage>(EActorType::EAT_Billboard, Transform);

		Transform.Translation.x = -Offset * FDXMath::RandF();
		Transform.Translation.z = -Offset * FDXMath::RandF();
		SpawnActor<AFoliage>(EActorType::EAT_Billboard, Transform);
	}*/
}
