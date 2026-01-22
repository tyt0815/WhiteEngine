#include "WESplineProjectileDemo.h"
#include "GameFramework/GameCore.h"

#include "Component/StaticMeshComponent.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/SplineComponent.h"
#include "Component/BoxComponent.h"

CREATE_APPLICATION(WSplineProjectileDemoWorld)

AHitReactor::AHitReactor()
{
	mHitBoxComp = CreateComponent<WBoxComponent>();
	SetRootComponent(mHitBoxComp);
	if (auto HitBox = mHitBoxComp.lock())
	{
		HitBox->ActivatePhysicBody();
		HitBox->GenerateOverlapEvent();
		HitBox->SetExtent(XMFLOAT3(.33f, .33f, .33f));
		HitBox->SetMotionType(EMotionType::Kinematic);
		HitBox->SetObjectChannel(EObjectChannel::EOC_Moving);
	}

	mSMComp = CreateComponent<WStaticMeshComponent>();
	if (auto SMComp = mSMComp.lock())
	{
		SMComp->SetupAttachment(GetRootComponent());
		SMComp->SetStaticMesh(FStaticMeshManager::GetInstance()->GetStaticMesh(EStaticMeshType::ESMT_ScuffedGoldBox));
		SMComp->SetLocalScale(XMFLOAT3(0.6f, 0.6f, 0.6f));
	}
}

void AHitReactor::BeginPlay()
{
	Super::BeginPlay();
}

void AHitReactor::AddTeleportTargets(const std::vector<TWeakPtr<WSplineComponent>>& NewTargets)
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

	if (auto Spline = mTeleportTargets[i].lock())
	{
		float InputKey = FDXMath::RandF() * (Spline->GetControllPointNum() - 1);

		SetActorLocation(Spline->GetWorldTransformAtSplineInputKey(InputKey).Translation);
	}
}

AProjectileActor::AProjectileActor()
{
	mProjectileMovementComponent = CreateComponent<WProjectileMovementComponent>();
	if (auto Comp = mProjectileMovementComponent.lock())
	{
		Comp->mVelocity = XMFLOAT3(0, 0, 10);
		Comp->SetLifeSpan(10.0f);
	}
}

XMFLOAT3 AProjectileActor::GetVelocity()
{
	return !mProjectileMovementComponent.expired() ? mProjectileMovementComponent.lock()->mVelocity : XMFLOAT3();
}

ABullet::ABullet()
{
	mHitBoxComp = CreateComponent<WBoxComponent>();
	SetRootComponent(mHitBoxComp);
	if (auto HitBoxComp = mHitBoxComp.lock())
	{
		HitBoxComp->ActivatePhysicBody();
		HitBoxComp->SetExtent(XMFLOAT3(.15f, .15f, .5));
		HitBoxComp->SetMotionType(EMotionType::Kinematic);
		HitBoxComp->SetObjectChannel(EObjectChannel::EOC_Moving);
	}


	mStaticMesh = CreateComponent<WStaticMeshComponent>();
	if (auto StaticMesh = mStaticMesh.lock())
	{
		StaticMesh->SetStaticMesh(FStaticMeshManager::GetInstance()->GetStaticMesh(EStaticMeshType::ESMT_MetalCylinder));
		StaticMesh->SetLocalRotation(XMFLOAT3(90, 0, 0));
		StaticMesh->SetupAttachment(GetRootComponent());
	}
}

void ABullet::BeginPlay()
{
	Super::BeginPlay();

	mHitBoxComp.lock()->mOnBeginOverlapDelegate.Bind(this, &ABullet::OnOverlap);
}

void ABullet::OnOverlap(TWeakPtr<WPhysicsComponent> Other, XMFLOAT3 ImpactPoint)
{
	if (auto OtherComp = Other.lock())
	{
		if (auto Reactor = Cast<AHitReactor>(OtherComp->GetOwner().lock()))
		{
			Reactor->RandomTeleport();
		}
		Destroy();
	}
}

ARingProjectile::ARingProjectile()
{
	mStaticMesh = CreateComponent<WStaticMeshComponent>();
	if (auto StaticMesh = mStaticMesh.lock())
	{
		StaticMesh->SetupAttachment(GetRootComponent());
		StaticMesh->SetStaticMesh(FStaticMeshManager::GetInstance()->GetStaticMesh(EStaticMeshType::ESMT_MetalRing));
		StaticMesh->SetLocalRotation(XMFLOAT3(90, 0, 0));
	}

	if (auto ProjectileMovementComponent = mProjectileMovementComponent.lock())
	{
		ProjectileMovementComponent->mVelocity = XMFLOAT3(0, 0, 5);
		ProjectileMovementComponent->SetLifeSpan(10.0f);
	}


	// Ring의 HitBox를 설정한다. 여러개의 BoxComponent로 Ring 모양의 히트박스를 만든다.
	const int SegmentCoef = 5;
	const int NumSegments = 4 * SegmentCoef;	// NumSegments는 4의 배수로 설정한다.
	const float AngleDelta = FDXMath::Pi / 2 / (NumSegments / 4);
	const float Radius = 2;
	const XMFLOAT3 Extent = { 0.25f, 2.0f / (NumSegments / 4), 0.25f };

	float Angle = 0;
	for (int i = 0; i < NumSegments; ++i, Angle += AngleDelta)
	{
		TWeakPtr<WBoxComponent> HitBoxComponent = CreateComponent<WBoxComponent>();
		if (auto HitBox = HitBoxComponent.lock())
		{
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
}

void ARingProjectile::BeginPlay()
{
	Super::BeginPlay();

	for (TWeakPtr<WBoxComponent> HitBoxWeak : mHitBoxes)
	{
		if (auto HitBox = HitBoxWeak.lock())
		{
			HitBox->mOnBeginOverlapDelegate.Bind(this, &ARingProjectile::OnOverlapEvent);
		}
	}
}

void ARingProjectile::Tick_PostPhysics(float Delta)
{
	Super::Tick_PostPhysics(Delta);
}

void ARingProjectile::OnOverlapEvent(TWeakPtr<WPhysicsComponent> Other, XMFLOAT3 ImpactPoint)
{
	if (Other.expired())
	{
		return;
	}
	TSharedPtr<WPhysicsComponent> sp = Other.lock();

	for (auto ActorToIgnore : ActorsToIgnore)
	{
		if (!ActorToIgnore.expired() && ActorToIgnore.lock().get() == sp->GetOwner().lock().get())
		{
			return;
		}
	}

	ActorsToIgnore.push_back(sp->GetOwner());

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
	mSplineBullet.Spline = CreateComponent<WSplineComponent>();
	if (auto Spline = mSplineBullet.Spline.lock())
	{
		Spline->SetupAttachment(GetRootComponent());
		Spline->LoadSplineFromAsset(L"SDA_Spiral");
	}
	
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

	if (auto Spline = mSplineBullet.Spline.lock())
	{
		for (int i = 0; i < mSplineBullet.Bullets.size(); ++i)
		{
			if (auto Bullet = mSplineBullet.Bullets[i].lock())
			{
				float& Dist = mSplineBullet.BulletDistance[i];

				float SpeedCoef = Spline->GetCustomProperty2AtDistanceAlongSpline(Dist);

				Dist = min(Dist + Delta * Bullet->GetVelocity().z * SpeedCoef, Spline->GetSplineLength());

				Bullet->SetActorTransform(Spline->GetWorldTransformAtDistanceAlongSpline(Dist));

				if (Dist >= Spline->GetSplineLength())
				{
					mSplineBullet.RemoveAt(i);
					--i;
					continue;
				}
			}
			else
			{
				mSplineBullet.RemoveAt(i);
				--i;
				continue;
			}
		}
	}
}

AWaveBulletSpanwer::AWaveBulletSpanwer()
{
	mSplines.resize(4);

	XMFLOAT3 LocationOffset = { 0.5f, 0, 0 };
	XMFLOAT3 RotationOffset = {0, 20, 90};
	float k = mSplines.size() / 2.f - 0.5f;
	for (int i = 0; i < mSplines.size(); ++i)
	{
		float v = (i - k);
		mSplines[i].Spline = CreateComponent<WSplineComponent>();
		if (auto Spline = mSplines[i].Spline.lock())
		{
			Spline->SetupAttachment(GetRootComponent());
			Spline->LoadSplineFromAsset(L"SDA_Wave");
			Spline->SetLocalLocation(XMFLOAT3(LocationOffset.x * v, LocationOffset.y * v, LocationOffset.z * v));
			Spline->SetLocalRotation(XMFLOAT3(RotationOffset.x * v, RotationOffset.y * v, RotationOffset.z * v));
		}
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

std::vector<TWeakPtr<WSplineComponent>> AWaveBulletSpanwer::GetSplines() const
{
	std::vector<TWeakPtr<WSplineComponent>> Splines;
	for (const auto& s : mSplines)
	{
		Splines.push_back(s.Spline);
	}
	return Splines;
}

void AWaveBulletSpanwer::UpdateSplineBullet(FSplineBullet* SplineBullet, float Delta)
{
	if (auto Spline = SplineBullet->Spline.lock())
	{
		for (int i = 0; i < SplineBullet->Bullets.size(); ++i)
		{
			if (auto Bullet = SplineBullet->Bullets[i].lock())
			{

				float& Dist = SplineBullet->BulletDistance[i];

				float SpeedCoef = Spline->GetCustomProperty2AtDistanceAlongSpline(Dist);

				Dist = min(Dist + Delta * Bullet->GetVelocity().z * SpeedCoef, Spline->GetSplineLength());

				Bullet->SetActorTransform(Spline->GetWorldTransformAtDistanceAlongSpline(Dist));

				if (Dist >= Spline->GetSplineLength())
				{
					SplineBullet->RemoveAt(i);
					Bullet->Destroy();
					--i;
					continue;
				}
			}
			else
			{
				SplineBullet->RemoveAt(i);
				--i;
				continue;
			}
		}
	}
}

TWeakPtr<ABullet> ASplineBulletSpawner::FSplineBullet::SpawnBullet(WWorld* World)
{
	TWeakPtr<ABullet> Bullet = World->SpawnActor<ABullet>();
	Bullets.emplace_back(Bullet);
	BulletDistance.emplace_back(0.0f);
	return Bullet;
}

void ASplineBulletSpawner::FSplineBullet::RemoveAt(UINT i)
{
	Bullets[i] = Bullets.back();
	Bullets.pop_back();
	BulletDistance[i] = BulletDistance.back();
	BulletDistance.pop_back();
}

ARingProjectileSpawner::ARingProjectileSpawner()
{
	mSpline = CreateComponent<WSplineComponent>();
	if (auto Spline = mSpline.lock())
	{
		Spline->SetupAttachment(GetRootComponent());
		Spline->LoadSplineFromAsset(L"SDA_RingPath");
	}
}

void ARingProjectileSpawner::Tick_PostPhysics(float Delta)
{
	Super::Tick_PostPhysics(Delta);

	mCoolDown -= Delta;

	WWorld* World = GetWorld();
	if (mCoolDown < 0)
	{
		mRings.emplace_back(World->SpawnActor<ARingProjectile>());
		mDists.emplace_back(0.0f);
		mCoolDown = mCoolTime;
	}

	if (auto Spline = mSpline.lock())
	{
		for (int i = 0; i < mRings.size(); ++i)
		{
			if (auto Ring = mRings[i].lock())
			{
				float& Dist = mDists[i];

				float SpeedCoef = Spline->GetCustomProperty2AtDistanceAlongSpline(Dist);

				Dist = min(Dist + Delta * Ring->GetVelocity().z * SpeedCoef, Spline->GetSplineLength());

				Ring->SetActorTransform(Spline->GetWorldTransformAtDistanceAlongSpline(Dist));

				if (Dist >= Spline->GetSplineLength())
				{
					mRings[i] = mRings.back();
					mRings.pop_back();
					mDists[i] = mDists.back();
					mDists.pop_back();
					--i;
					continue;
				}
			}
			else
			{
				mRings[i] = mRings.back();
				mRings.pop_back();
				mDists[i] = mDists.back();
				mDists.pop_back();
				--i;
				continue;
			}
		}
	}
}


WSplineProjectileDemoWorld::WSplineProjectileDemoWorld()
{
	;
	if (auto SpiralSpawner = SpawnActor<ASpiralBulletSpawner>().lock())
	{
		SpiralSpawner->SetActorLocation(XMFLOAT3(5, -2, 10));
		SpiralSpawner->SetActorRotation(XMFLOAT3(0, 90, 0));
	}

	
	if (auto WaveSpawner = SpawnActor<AWaveBulletSpanwer>().lock())
	{
		WaveSpawner->SetActorLocation(XMFLOAT3(-5, -2, 15));
		WaveSpawner->SetActorRotation(XMFLOAT3(0, 180, 0));

		
		if (auto BulletHitReactor = SpawnActor<AHitReactor>().lock())
		{
			BulletHitReactor->AddTeleportTargets(WaveSpawner->GetSplines());
			BulletHitReactor->RandomTeleport();
		}
	}

	
	if (auto RingSpawner = SpawnActor<ARingProjectileSpawner>().lock())
	{
		RingSpawner->SetActorLocation(XMFLOAT3(-5, -2, 20));
		RingSpawner->SetActorRotation(XMFLOAT3(0, 90, 0));
	}

	if (auto RingHitReactor = SpawnActor<AHitReactor>().lock())
	{
		RingHitReactor->SetActorLocation(XMFLOAT3(5, -2, 20));
	}
}