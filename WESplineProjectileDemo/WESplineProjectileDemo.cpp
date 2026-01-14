#include "WESplineProjectileDemo.h"
#include "GameFramework/GameAppImpl.h"

#include "Component/StaticMeshComponent.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/SplineComponent.h"

CREATE_APPLICATION_BY_WORLD(WSplineProjectileDemoWorld)

ABullet::ABullet()
{
	WSceneComponent* DummyRoot = CreateComponent<WSceneComponent>();
	SetRootComponent(DummyRoot);
	mStaticMesh = CreateComponent<WStaticMeshComponent>();
	mStaticMesh->SetStaticMesh(FStaticMeshManager::GetInstance()->GetStaticMesh(EStaticMeshType::ESMT_MetalCylinder));
	mStaticMesh->SetLocalRotation(XMFLOAT3(90, 0, 0));
	mStaticMesh->SetupAttachment(GetRootComponent());
	mProjectileMovementComponent = CreateComponent<WProjectileMovementComponent>();
	mProjectileMovementComponent->mVelocity = XMFLOAT3(0, 0, 10);
	mProjectileMovementComponent->SetLifeSpan(10.0f);
}

XMFLOAT3 ABullet::GetVelocity()
{
	return mProjectileMovementComponent->mVelocity;
}

ASpiralBulletSpawner::ASpiralBulletSpawner()
{
	WSceneComponent* RootComponent = CreateComponent<WSceneComponent>();
	SetRootComponent(RootComponent);
	mSpiralSpline = CreateComponent<WSplineComponent>();
	mSpiralSpline->SetupAttachment(GetRootComponent());
	mSpiralSpline->LoadSplineFromAsset(L"SDA_Spiral");
}

void ASpiralBulletSpawner::Tick_PostPhysics(float Delta)
{
	ABulletSpawner::Tick_PostPhysics(Delta);

	mCoolDown -= Delta;

	WWorld* World = GetWorld();
	if (mCoolDown < 0)
	{
		ABullet* Bullet = World->SpawnActor<ABullet>();
		mBullets.Add(Bullet);
		mBulletDistance.Add(0.0f);
		mCoolDown = mCoolTime;
	}

	for (int i = 0; i < mBullets.Size(); ++i)
	{
		if (!World->IsValidActor(mBullets[i]))
		{
			mBullets.RemoveAt(i);
			mBulletDistance.RemoveAt(i);
			--i;
			continue;
		}

		ABullet* Bullet = mBullets[i];
		float& Dist = mBulletDistance[i];

		float SpeedCoef = mSpiralSpline->GetCustomProperty2AtDistanceAlongSpline(Dist);

		Dist = min(Dist + Delta * Bullet->GetVelocity().z * SpeedCoef, mSpiralSpline->GetSplineLength());

		Bullet->SetActorTransform(mSpiralSpline->GetWorldTransformAtDistanceAlongSpline(Dist));

		if (Dist >= mSpiralSpline->GetSplineLength())
		{
			mBullets.RemoveAt(i);
			mBulletDistance.RemoveAt(i);
			--i;
			continue;
		}
	}
}


WSplineProjectileDemoWorld::WSplineProjectileDemoWorld()
{
	ASpiralBulletSpawner* SpiralSpawner = SpawnActor<ASpiralBulletSpawner>();
	SpiralSpawner->SetActorLocation(XMFLOAT3(5, 2, 10));
}