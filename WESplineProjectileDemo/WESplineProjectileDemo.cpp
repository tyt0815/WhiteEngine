#include "WESplineProjectileDemo.h"
#include "GameFramework/GameAppImpl.h"

#include "Component/StaticMeshComponent.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/SplineComponent.h"
#include "Component/BoxComponent.h"

CREATE_APPLICATION_BY_WORLD(WSplineProjectileDemoWorld)

AHitReactor::AHitReactor()
{
	mHitBoxComp = CreateComponent<WBoxComponent>();
	SetRootComponent(mHitBoxComp);
	mHitBoxComp->ActivatePhysicBody();
	mHitBoxComp->GenerateOverlapEvent();
	mHitBoxComp->SetExtent(XMFLOAT3(.33f, .33f, .33f));
	mHitBoxComp->SetMotionType(EMotionType::Kinematic);
	mHitBoxComp->SetObjectChannel(EObjectChannel::EOC_Moving);

	mSMComp = CreateComponent<WStaticMeshComponent>();
	mSMComp->SetupAttachment(GetRootComponent());
	mSMComp->SetStaticMesh(FStaticMeshManager::GetInstance()->GetStaticMesh(EStaticMeshType::ESMT_ScuffedGoldBox));
	mSMComp->SetLocalScale(XMFLOAT3(0.6f, 0.6f, 0.6f));
}

void AHitReactor::BeginPlay()
{
	Super::BeginPlay();
}

void AHitReactor::AddTeleportTargets(const std::vector<WSplineComponent*>& NewTargets)
{
	mTeleportTargets.insert(mTeleportTargets.begin(), NewTargets.begin(), NewTargets.end());
}

void AHitReactor::RandomTeleport()
{
	if (mTeleportTargets.size() < 1)
	{
		return;
	}

	int i = FDXMath::Rand(0, int(mTeleportTargets.size() - 1));

	WSplineComponent* Spline = mTeleportTargets[i];

	float InputKey = FDXMath::RandF() * (Spline->GetControllPointNum() - 1);

	SetActorLocation(Spline->GetWorldTransformAtSplineInputKey(InputKey).Translation);
}

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
	mHitBoxComp = CreateComponent<WBoxComponent>();
	SetRootComponent(mHitBoxComp);
	mHitBoxComp->ActivatePhysicBody();
	mHitBoxComp->SetExtent(XMFLOAT3(.15f, .15f, .5));
	mHitBoxComp->SetMotionType(EMotionType::Kinematic);
	mHitBoxComp->SetObjectChannel(EObjectChannel::EOC_Moving);

	mStaticMesh = CreateComponent<WStaticMeshComponent>();
	mStaticMesh->SetStaticMesh(FStaticMeshManager::GetInstance()->GetStaticMesh(EStaticMeshType::ESMT_MetalCylinder));
	mStaticMesh->SetLocalRotation(XMFLOAT3(90, 0, 0));
	mStaticMesh->SetupAttachment(GetRootComponent());
}

void ABullet::BeginPlay()
{
	Super::BeginPlay();

	mHitBoxComp->mOnBeginOverlapDelegate.Bind(this, &ABullet::OnOverlap);
}

void ABullet::OnOverlap(WPrimitiveComponent* Other, XMFLOAT3 ImpactPoint)
{
	if (AHitReactor* Reactor = dynamic_cast<AHitReactor*>(Other->GetOwner()))
	{
		Reactor->RandomTeleport();
	}
	Destroy();
}

ARingProjectile::ARingProjectile()
{
	WSceneComponent* DummyRoot = CreateComponent<WSceneComponent>();
	SetRootComponent(DummyRoot);
	mStaticMesh = CreateComponent<WStaticMeshComponent>();
	mStaticMesh->SetupAttachment(GetRootComponent());
	mStaticMesh->SetStaticMesh(FStaticMeshManager::GetInstance()->GetStaticMesh(EStaticMeshType::ESMT_MetalRing));
	mStaticMesh->SetLocalRotation(XMFLOAT3(90, 0, 0));

	mProjectileMovementComponent->mVelocity = XMFLOAT3(0, 0, 5);
	mProjectileMovementComponent->SetLifeSpan(10.0f);


	// Ring의 HitBox를 설정한다. 여러개의 BoxComponent로 Ring 모양의 히트박스를 만든다.
	const int SegmentCoef = 5;
	const int NumSegments = 4 * SegmentCoef;	// NumSegments는 4의 배수로 설정한다.
	const float AngleDelta = FDXMath::Pi / 2 / (NumSegments / 4);
	const float Radius = 2;
	const XMFLOAT3 Extent = { 0.25f, 2.0f / (NumSegments / 4), 0.25f };

	float Angle = 0;
	for (int i = 0; i < NumSegments; ++i, Angle += AngleDelta)
	{
		WBoxComponent* HitBox = CreateComponent<WBoxComponent>();
		HitBox->SetupAttachment(GetRootComponent());
		HitBox->ActivatePhysicBody();
		HitBox->SetExtent(Extent);
		HitBox->SetMotionType(EMotionType::Kinematic);
		HitBox->SetObjectChannel(EObjectChannel::EOC_Moving);

		float x = cosf(Angle) * Radius;
		float y = sinf(Angle) * Radius;
		HitBox->SetLocalLocation(XMFLOAT3(x, y, 0));
		HitBox->SetLocalRotation(XMFLOAT3(0, 0, Angle * (180 / FDXMath::Pi)));

		mHitBoxes.push_back(HitBox);
	}
}

void ARingProjectile::BeginPlay()
{
	Super::BeginPlay();
	for (WBoxComponent* HitBox : mHitBoxes)
	{
		HitBox->mOnBeginOverlapDelegate.Bind(this, &ARingProjectile::OnOverlapEvent);
	}
}

void ARingProjectile::Tick_PostPhysics(float Delta)
{
	Super::Tick_PostPhysics(Delta);
}

void ARingProjectile::OnOverlapEvent(WPrimitiveComponent* Other, XMFLOAT3 ImpactPoint)
{
	if (std::find(ActorsToIgnore.begin(), ActorsToIgnore.end(), Other->GetOwner()) != ActorsToIgnore.end())
	{
		return;
	}

	ActorsToIgnore.push_back(Other->GetOwner());

	static int ColorSelector = 0;
	const XMFLOAT4 DebugColors[] = {
		{1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1},
		{1, 1, 0, 1}, {1, 0, 1, 1}, {0, 1, 1, 1}
	};

	GetWorld()->DrawDebugLine(GetActorLocation(), ImpactPoint, DebugColors[ColorSelector], 3);
	ColorSelector = (ColorSelector + 1) % 6;
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

std::vector<WSplineComponent*> AWaveBulletSpanwer::GetSplines() const
{
	std::vector<WSplineComponent*> Splines;
	for (const auto& s : mSplines)
	{
		Splines.push_back(s.Spline);
	}
	return Splines;
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

ARingProjectileSpawner::ARingProjectileSpawner()
{
	WSceneComponent* RootComponent = CreateComponent<WSceneComponent>();
	SetRootComponent(RootComponent);

	mSpline = CreateComponent<WSplineComponent>();
	mSpline->SetupAttachment(RootComponent);
	mSpline->LoadSplineFromAsset(L"SDA_RingPath");
}

void ARingProjectileSpawner::Tick_PostPhysics(float Delta)
{
	Super::Tick_PostPhysics(Delta);

	mCoolDown -= Delta;

	WWorld* World = GetWorld();
	if (mCoolDown < 0)
	{
		mRings.Add(World->SpawnActor<ARingProjectile>());
		mDists.Add(0.0f);
		mCoolDown = mCoolTime;
	}

	for (int i = 0; i < mRings.Size(); ++i)
	{

		if (!World->IsValidActor(mRings[i]))
		{
			mRings.RemoveAt(i);
			mDists.RemoveAt(i);
			--i;
			continue;
		}

		ARingProjectile* Ring = mRings[i];
		float& Dist = mDists[i];

		float SpeedCoef = mSpline->GetCustomProperty2AtDistanceAlongSpline(Dist);

		Dist = min(Dist + Delta * Ring->GetVelocity().z * SpeedCoef, mSpline->GetSplineLength());

		Ring->SetActorTransform(mSpline->GetWorldTransformAtDistanceAlongSpline(Dist));

		if (Dist >= mSpline->GetSplineLength())
		{
			mRings.RemoveAt(i);
			mDists.RemoveAt(i);
			--i;
			continue;
		}
	}
}


WSplineProjectileDemoWorld::WSplineProjectileDemoWorld()
{
	ASpiralBulletSpawner* SpiralSpawner = SpawnActor<ASpiralBulletSpawner>();
	SpiralSpawner->SetActorLocation(XMFLOAT3(5, -2, 10));
	SpiralSpawner->SetActorRotation(XMFLOAT3(0, 90, 0));

	AWaveBulletSpanwer* WaveSpawner = SpawnActor<AWaveBulletSpanwer>();
	WaveSpawner->SetActorLocation(XMFLOAT3(-5, -2, 15));
	WaveSpawner->SetActorRotation(XMFLOAT3(0, 180, 0));

	AHitReactor* BulletHitReactor = SpawnActor<AHitReactor>();
	auto WaveSplines = WaveSpawner->GetSplines();
	BulletHitReactor->AddTeleportTargets(WaveSpawner->GetSplines());
	BulletHitReactor->RandomTeleport();

	ARingProjectileSpawner* RingSpawner = SpawnActor <ARingProjectileSpawner>();
	RingSpawner->SetActorLocation(XMFLOAT3(-5, -2, 20));
	RingSpawner->SetActorRotation(XMFLOAT3(0, 90, 0));

	AHitReactor* RingHitReactor = SpawnActor<AHitReactor>();
	RingHitReactor->SetActorLocation(XMFLOAT3(5, -2, 20));
}