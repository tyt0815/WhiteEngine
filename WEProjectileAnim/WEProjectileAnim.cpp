#include "WEProjectileAnim.h"
#include "GameFramework/GameAppImpl.h"
#include "Component/ObjectAnimComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Utility/Timer.h"
#include "GUI/GUICore.h"

CREATE_APPLICATION_BY_WORLD(WProjectileAnimWorld)

AProjectile::AProjectile()
{
	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(mStaticMeshComp);
	if (auto Comp = mStaticMeshComp.lock())
	{
		Comp->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_ScuffedGoldSphere));
	}
}

AProjectileAnimActor::AProjectileAnimActor()
{
	mObjectAnimComp = CreateComponent<WObjectAnimComponent>();
	if (auto ObjectAnimComp = mObjectAnimComp.lock())
	{
		ObjectAnimComp->SetupAttachment(GetRootComponent());

		ObjectAnimComp->LoadKeyframesFromOADAsset(L"OAD_Large");
	}
}

void AProjectileAnimActor::BeginPlay()
{
	Super::BeginPlay();

	mProj = GetWorld()->SpawnActor<AProjectile>();
}

void AProjectileAnimActor::Tick_PostPhysics(float Delta)
{
	Super::Tick_PostPhysics(Delta);

	mElapsedTime += Delta;

	if (auto ObjectAnimComp = mObjectAnimComp.lock())
	{
		mElapsedTime = fmodf(mElapsedTime, ObjectAnimComp->GetDuration());

		if (auto Proj = mProj.lock())
		{
			FTransform Transform = ObjectAnimComp->SampleAnimWorldTransformBySecond(mElapsedTime);
			Proj->SetActorTransform(Transform);
		}
	}
}

WProjectileAnimWorld::WProjectileAnimWorld()
{
	int ProjNum = 100;
	for (int i = 0; i < ProjNum; ++i)
	{
		if (auto Projectile = SpawnActor<AProjectileAnimActor>().lock())
		{
			float r = 360.0f / ProjNum * i;
			XMFLOAT3 Rot(r, r, r);
			Projectile->SetActorRotation(Rot);
		}
	}
}