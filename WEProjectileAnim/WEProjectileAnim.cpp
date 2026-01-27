#include "WEProjectileAnim.h"
#include "GameFramework/GameCore.h"
#include "Component/ObjectAnimComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/BoxComponent.h"
#include "Utility/Timer.h"
#include "GUI/GUICore.h"

CREATE_APPLICATION(WProjectileAnimWorld)

ARing::ARing()
{
	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	if (auto Comp = mStaticMeshComp.lock())
	{
		Comp->SetupAttachment(GetRootComponent());
		Comp->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_MetalRing));
	}
}

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

	mObjectAnimComp = CreateComponent<WObjectAnimComponent>();
	if (auto ObjectAnimComp = mObjectAnimComp.lock())
	{
		ObjectAnimComp->SetupAttachment(GetRootComponent());

		ObjectAnimComp->LoadKeyframesFromOADAsset(L"OAD_LeftRight");

		mProjAnimSampler = ObjectAnimComp->GetObjectAnimSampler("Cube");
	}

	mProjComp = CreateComponent<WProjectileMovementComponent>();
	if (auto Comp = mProjComp.lock())
	{
		Comp->mVelocity = XMFLOAT3(1, 0, 0);
	}

	SetTickGroup(ETickGroup::ETG_PostPhysics, ETickPriority::ETP_High);
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	mRing = GetWorld()->SpawnActor<ARing>();
}

void AProjectile::Tick(float Delta)
{
	Super::Tick(Delta);

	mElapsedTime += Delta;

	if (auto ObjectAnimComp = mObjectAnimComp.lock())
	{
		mElapsedTime = fmodf(mElapsedTime, ObjectAnimComp->GetDuration());

		if (auto Ring = mRing.lock())
		{
			if (mElapsedTime > 0.5f)
			{
				float a = mElapsedTime;
			}
			FTransform Transform = ObjectAnimComp->SampleAnimWorldTransformBySecond(mProjAnimSampler, mElapsedTime);
			Ring->SetActorTransform(Transform);
		}
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

	SetTickGroup(ETickGroup::ETG_PostPhysics, ETickPriority::ETP_High);
}

void AProjectileAnimActor::BeginPlay()
{
	Super::BeginPlay();

	mProjs[0] = GetWorld()->SpawnActor<AProjectile>();
	mProjs[1] = GetWorld()->SpawnActor<AProjectile>();
	mProjs[2] = GetWorld()->SpawnActor<AProjectile>();
}

void AProjectileAnimActor::Tick(float Delta)
{
	Super::Tick(Delta);

	mElapsedTime += Delta;

	if (auto ObjectAnimComp = mObjectAnimComp.lock())
	{
		if (mElapsedTime > ObjectAnimComp->GetDuration())
		{
			mElapsedTime = 0;

			mProjs[0] = GetWorld()->SpawnActor<AProjectile>();
			mProjs[1] = GetWorld()->SpawnActor<AProjectile>();
			mProjs[2] = GetWorld()->SpawnActor<AProjectile>();
		}

		for (int i = 0; i < 3; ++i)
		{
			if (auto Proj = mProjs[i].lock())
			{
				FTransform Transform = ObjectAnimComp->SampleAnimWorldTransformBySecond(mProjAnimSamplers[i], mElapsedTime);
				Proj->SetActorTransform(Transform);
			}
		}
	}
}

void WProjectileAnimWorld::BeginPlay()
{
	Super::BeginPlay();

	int Num = 1;
	for (int i = 0; i < Num; ++i)
	{
		if (auto Projectile = SpawnActor<AProjectileAnimActor>().lock())
		{
			Projectile->SetActorLocation(XMFLOAT3(0, (float)i, 0));
			mProjs.push_back(Projectile);
		}
	}
}
