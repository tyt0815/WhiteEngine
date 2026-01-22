#include "WEProjectileAnim.h"
#include "GameFramework/GameAppImpl.h"
#include "Component/ObjectAnimComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/BoxComponent.h"
#include "Utility/Timer.h"
#include "GUI/GUICore.h"

CREATE_APPLICATION_BY_WORLD(WProjectileAnimWorld)

AProjectile::AProjectile()
{
	//mBoxCollision = CreateComponent<WBoxComponent>();
	//SetRootComponent(mBoxCollision);
	//if (auto Box = mBoxCollision.lock())
	//{
	//	Box->ActivatePhysicBody();
	//	Box->GenerateOverlapEvent();
	//	Box->SetExtent(XMFLOAT3(.5f, .5f, .5f));
	//	Box->SetMotionType(EMotionType::Kinematic);
	//	Box->SetObjectChannel(EObjectChannel::EOC_Moving);
	//}

	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	if (auto Comp = mStaticMeshComp.lock())
	{
		Comp->SetupAttachment(GetRootComponent());
		Comp->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_ScuffedGoldBox));
	}
}

AProjectileAnimActor::AProjectileAnimActor()
{
	mObjectAnimComp = CreateComponent<WObjectAnimComponent>();
	if (auto ObjectAnimComp = mObjectAnimComp.lock())
	{
		ObjectAnimComp->SetupAttachment(GetRootComponent());

		ObjectAnimComp->LoadKeyframesFromOADAsset(L"OAD_MultiAnim");

		mProjAnimSamplers[0] = ObjectAnimComp->GetObjectAnimSampler("Proj1");
		mProjAnimSamplers[1] = ObjectAnimComp->GetObjectAnimSampler("Proj2");
		mProjAnimSamplers[2] = ObjectAnimComp->GetObjectAnimSampler("Proj3");
	}
}

void AProjectileAnimActor::BeginPlay()
{
	Super::BeginPlay();

	mProjs[0] = GetWorld()->SpawnActor<AProjectile>();
	mProjs[1] = GetWorld()->SpawnActor<AProjectile>();
	mProjs[2] = GetWorld()->SpawnActor<AProjectile>();
}

void AProjectileAnimActor::Tick_PostPhysics(float Delta)
{
	Super::Tick_PostPhysics(Delta);

	mElapsedTime += Delta;

	if (auto ObjectAnimComp = mObjectAnimComp.lock())
	{
		mElapsedTime = fmodf(mElapsedTime, ObjectAnimComp->GetDuration());

		for (int i = 0; i < 3; ++i)
		{
			if (auto Proj = mProjs[i].lock())
			{
				if (mElapsedTime > 0.5f)
				{
					float a = mElapsedTime;
				}
				FTransform Transform = ObjectAnimComp->SampleAnimWorldTransformBySecond(mProjAnimSamplers[i], mElapsedTime);
				Proj->SetActorTransform(Transform);
			}
		}
	}
}

void AProjSpawner::Tick_PrePhysics(float Delta)
{
	Super::Tick_PrePhysics(Delta);
}

WProjectileAnimWorld::WProjectileAnimWorld()
{
	if (auto Projectile = SpawnActor<AProjectileAnimActor>().lock())
	{
		mProjs.push_back(Projectile);
	}
}