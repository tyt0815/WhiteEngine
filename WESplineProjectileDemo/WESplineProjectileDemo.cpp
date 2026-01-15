#include "WESplineProjectileDemo.h"
#include "GameFramework/GameAppImpl.h"

#include "Component/StaticMeshComponent.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/SplineComponent.h"

CREATE_APPLICATION_BY_WORLD(WSplineProjectileDemoWorld)

AProjectileActor::AProjectileActor()
{
	mProjectileMovementComponent = CreateComponent<WProjectileMovementComponent>();
	mProjectileMovementComponent->mVelocity = XMFLOAT3(0, 0, 10);
	mProjectileMovementComponent->SetLifeSpan(10.0f);
}

XMFLOAT3 AProjectileActor::GetVelocity()
{
	return mProjectileMovementComponent->mVelocity;
}

ABullet::ABullet()
{
	WSceneComponent* DummyRoot = CreateComponent<WSceneComponent>();
	SetRootComponent(DummyRoot);
	mStaticMesh = CreateComponent<WStaticMeshComponent>();
	mStaticMesh->SetStaticMesh(FStaticMeshManager::GetInstance()->GetStaticMesh(EStaticMeshType::ESMT_MetalCylinder));
	mStaticMesh->SetLocalRotation(XMFLOAT3(90, 0, 0));
	mStaticMesh->SetupAttachment(GetRootComponent());
}

ARingProjectile::ARingProjectile()
{
	WSceneComponent* DummyRoot = CreateComponent<WSceneComponent>();
	SetRootComponent(DummyRoot);
	mStaticMesh = CreateComponent<WStaticMeshComponent>();
	mStaticMesh->SetupAttachment(GetRootComponent());
	mStaticMesh->SetStaticMesh(FStaticMeshManager::GetInstance()->GetStaticMesh(EStaticMeshType::ESMT_MetalRing));
	mStaticMesh->SetLocalRotation(XMFLOAT3(90, 0, 0));

	mProjectileMovementComponent->mVelocity = XMFLOAT3(0, 0, 0);
	mProjectileMovementComponent->SetLifeSpan(1000.0f);
}


ASpiralBulletSpawner::ASpiralBulletSpawner()
{
	WSceneComponent* RootComponent = CreateComponent<WSceneComponent>();
	SetRootComponent(RootComponent);
	
	mSplineBullet.Spline = CreateComponent<WSplineComponent>();
	mSplineBullet.Spline->SetupAttachment(GetRootComponent());
	mSplineBullet.Spline->LoadSplineFromAsset(L"SDA_Spiral");
}

void ASpiralBulletSpawner::Tick_PostPhysics(float Delta)
{
	ASplineBulletSpawner::Tick_PostPhysics(Delta);

	mCoolDown -= Delta;

	WWorld* World = GetWorld();
	if (mCoolDown < 0)
	{
		mSplineBullet.SpawnBullet(World);
		mCoolDown = mCoolTime;
	}

	for (int i = 0; i < mSplineBullet.Bullets.Size(); ++i)
	{
		
		if (!World->IsValidActor(mSplineBullet.Bullets[i]))
		{
			mSplineBullet.RemoveAt(i);
			--i;
			continue;
		}

		ABullet* Bullet = mSplineBullet.Bullets[i];
		float& Dist = mSplineBullet.BulletDistance[i];

		float SpeedCoef = mSplineBullet.Spline->GetCustomProperty2AtDistanceAlongSpline(Dist);

		Dist = min(Dist + Delta * Bullet->GetVelocity().z * SpeedCoef, mSplineBullet.Spline->GetSplineLength());

		Bullet->SetActorTransform(mSplineBullet.Spline->GetWorldTransformAtDistanceAlongSpline(Dist));

		if (Dist >= mSplineBullet.Spline->GetSplineLength())
		{
			mSplineBullet.RemoveAt(i);
			--i;
			continue;
		}
	}
}

AWaveBulletSpanwer::AWaveBulletSpanwer()
{
	WSceneComponent* RootComponent = CreateComponent<WSceneComponent>();
	SetRootComponent(RootComponent);

	mSplines.resize(4);

	XMFLOAT3 LocationOffset = { 0.5f, 0, 0 };
	XMFLOAT3 RotationOffset = {0, 20, 90};
	float k = mSplines.size() / 2.f - 0.5f;
	for (int i = 0; i < mSplines.size(); ++i)
	{
		float v = (i - k);
		mSplines[i].Spline = CreateComponent<WSplineComponent>();
		mSplines[i].Spline->SetupAttachment(GetRootComponent());
		mSplines[i].Spline->LoadSplineFromAsset(L"SDA_Wave");
		mSplines[i].Spline->SetLocalLocation(XMFLOAT3(LocationOffset.x * v, LocationOffset.y * v, LocationOffset.z * v));
		mSplines[i].Spline->SetLocalRotation(XMFLOAT3(RotationOffset.x * v, RotationOffset.y * v, RotationOffset.z * v));
	}
}

void AWaveBulletSpanwer::Tick_PostPhysics(float Delta)
{
	Super::Tick_PostPhysics(Delta);

	mCoolDown -= Delta;
	if (mCoolDown <= 0)
	{
		mCoolDown = mCoolTime;

		for (auto& Spline : mSplines)
		{
			Spline.SpawnBullet(GetWorld());
		}
	}

	for (auto& Spline : mSplines)
	{
		UpdateSplineBullet(&Spline, Delta);
	}
}

void AWaveBulletSpanwer::UpdateSplineBullet(FSplineBullet* SplineBullet, float Delta)
{
	for (int i = 0; i < SplineBullet->Bullets.Size(); ++i)
	{
		if (!GetWorld()->IsValidActor(SplineBullet->Bullets[i]))
		{
			SplineBullet->RemoveAt(i);
			--i;
			continue;
		}

		ABullet* Bullet = SplineBullet->Bullets[i];
		float& Dist = SplineBullet->BulletDistance[i];

		float SpeedCoef = SplineBullet->Spline->GetCustomProperty2AtDistanceAlongSpline(Dist);

		Dist = min(Dist + Delta * Bullet->GetVelocity().z * SpeedCoef, SplineBullet->Spline->GetSplineLength());

		Bullet->SetActorTransform(SplineBullet->Spline->GetWorldTransformAtDistanceAlongSpline(Dist));

		if (Dist >= SplineBullet->Spline->GetSplineLength())
		{
			SplineBullet->RemoveAt(i);
			Bullet->Destroy();
			--i;
			continue;
		}
	}
}

ABullet* ASplineBulletSpawner::FSplineBullet::SpawnBullet(WWorld* World)
{
	ABullet* Bullet = World->SpawnActor<ABullet>();
	Bullets.Add(Bullet);
	BulletDistance.Add(0.0f);
	return Bullet;
}

void ASplineBulletSpawner::FSplineBullet::RemoveAt(UINT i)
{
	Bullets.RemoveAt(i);
	BulletDistance.RemoveAt(i);
}

WSplineProjectileDemoWorld::WSplineProjectileDemoWorld()
{
	ASpiralBulletSpawner* SpiralSpawner = SpawnActor<ASpiralBulletSpawner>();
	SpiralSpawner->SetActorLocation(XMFLOAT3(5, -2, 10));
	SpiralSpawner->SetActorRotation(XMFLOAT3(0, 90, 0));

	AWaveBulletSpanwer* WaveSpawner = SpawnActor<AWaveBulletSpanwer>();
	WaveSpawner->SetActorLocation(XMFLOAT3(-5, -2, 15));
	WaveSpawner->SetActorRotation(XMFLOAT3(0, 180, 0));

	ARingProjectile* Ring = SpawnActor<ARingProjectile>();
	Ring->SetActorLocation(XMFLOAT3(0, 0, 0));
}
