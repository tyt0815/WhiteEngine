#include "TestWorld.h"
#include "Actor/PhysicsBox.h"
#include "Actor/HitReactor.h"
#include "Actor/ProjectileBase.h"
#include <algorithm>

void WTestWorld::BeginPlay()
{
	//if (auto Player = SpawnActor<APlayerPawn>().lock())
	//{
	//	SetPlayer(Player);
	//}
	
	Super::BeginPlay();

	if (auto Proj = SpawnActor<AProjectileBase>(L"BP_Projectile").lock())
	{
		Proj->PlayAnimation();
	}

	XMFLOAT3 LocationOffset = XMFLOAT3(-40, 10, 40);
	XMFLOAT3 RotationOffset = XMFLOAT3(0, 90, 0);
	//mMissileSystems[0] = SpawnActor<AMissileSwarmSystem>();
	//if (auto MissileSystem = mMissileSystems[0].lock())
	//{
	//	MissileSystem->SetActorLocation(LocationOffset);
	//	MissileSystem->SetActorRotation(RotationOffset);

	//	mPlatform = SpawnActor<APhysicsBox>().lock();
	//	if (auto Box = mPlatform.lock())
	//	{
	//		Box->SetActorScale(XMFLOAT3(10, 1, 10));
	//		XMFLOAT3 Loc = CalcTargetOrigin(MissileSystem.get(), 80.0f);
	//		Loc.y += 20;
	//		Box->SetActorLocation(Loc);
	//	}
	//}

	LocationOffset = XMFLOAT3(-20, 0, 20);
	RotationOffset = XMFLOAT3(0, 90, 0);
	/*mMissileSystems[1] = SpawnActor<AMissileSwarmSystem>();
	if (auto MissileSystem = mMissileSystems[1].lock())
	{
		MissileSystem->SetActorLocation(LocationOffset);
		MissileSystem->SetActorRotation(RotationOffset);

		if (auto Box = SpawnActor<AHitReactor>().lock())
		{
			XMFLOAT3 Loc = CalcTargetOrigin(MissileSystem.get(), 40.0f);
			Box->SetActorLocation(Loc);
		}
		if (auto Box = SpawnActor<AHitReactor>().lock())
		{
			XMFLOAT3 Loc = CalcTargetOrigin(MissileSystem.get(), 40.0f);
			Loc.x -= 3;
			Box->SetActorLocation(Loc);
		}
		if (auto Box = SpawnActor<AHitReactor>().lock())
		{
			XMFLOAT3 Loc = CalcTargetOrigin(MissileSystem.get(), 40.0f);
			Loc.x -= 1.5f;
			Loc.y -= 3;
			Box->SetActorLocation(Loc);
		}
	}*/

	mElapsedTime = mDelay;
}

void WTestWorld::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	mElapsedTime += DeltaSecond;
}

XMFLOAT3 WTestWorld::CalcTargetOrigin(AActor* Actor, float TargetDistance)
{
	XMFLOAT3 TargetOrigin = Actor->GetActorLocation();
	XMFLOAT3 Forward = Actor->GetForwardVector();
	XMVECTOR TargetOriginV = XMLoadFloat3(&TargetOrigin);
	XMVECTOR ForwardV = XMLoadFloat3(&Forward);
	XMStoreFloat3(&TargetOrigin, XMVectorMultiplyAdd(ForwardV, XMVectorReplicate(TargetDistance), TargetOriginV));

	XMFLOAT3 TraceStart = TargetOrigin;
	XMFLOAT3 TraceEnd = TraceStart;
	TraceEnd.y -= 40;
	FHitResult Hit;
	TArray<AActor*> ActorsToIgnore;
	TArray<JPH::ObjectLayer> Layers;
	Layers.push_back(EObjectChannel::EOC_WorldStatic);
	LineTraceByObjectChannel(TraceStart, TraceEnd, ActorsToIgnore, Layers, Hit, true, 10.0f);

	if(!Hit.HitComponent.expired())
	{
		TargetOrigin = Hit.ImpactPoint;
	}

	return TargetOrigin;
}
