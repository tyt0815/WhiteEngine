#include "PlayerPawn.h"
#include "World/World.h"

APlayerPawn::APlayerPawn()
{
	SetTickGroup(ETickGroup::ETG_PostPhysics, ETickPriority::ETP_High);
}

void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	mMissileSwarmSystem = GetWorld()->SpawnActor<AMissileSwarmSystem>();
}

void APlayerPawn::SetupPlayerInput()
{
	Super::SetupPlayerInput();

	GetInputSystemManager()->BindKeyboardAction('1', this, &APlayerPawn::TriggerMissileSwarm);
}

void APlayerPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	if (auto MissileSystem = mMissileSwarmSystem.lock())
	{
		MissileSystem->SetActorTransform(GetActorTransform());

		if (mbMissileAiming)
		{
			WWorld* World = GetWorld();
			XMFLOAT3 TraceStart = GetActorLocation();
			const XMVECTOR TraceStartV = XMLoadFloat3(&TraceStart);
			XMFLOAT3 Forward = GetForwardVector();
			const XMVECTOR ForwardV = XMLoadFloat3(&Forward);
			XMFLOAT3 Right = GetRightVector();
			XMVECTOR RightV = XMLoadFloat3(&Right);
			XMFLOAT3 TraceEnd;
			XMStoreFloat3(&TraceEnd, XMVectorAdd(TraceStartV, XMVectorScale(ForwardV, 100.0f)));
			XMFLOAT3 Orientaion = GetActorRotation();

			TArray<AActor*> ActorsToIgnore;
			FHitResult HitResult;
			World->BoxTrace(
				TraceStart, TraceEnd,
				XMFLOAT3(0, 1.0f, 1.0f),
				Orientaion,
				ActorsToIgnore,
				HitResult,
				true,
				0
			);

			MissileSystem->SetTargetMarkersLocation(HitResult.ImpactPoint, Right, 1);
		}
	}	
}

void APlayerPawn::TriggerMissileSwarm(float Delta)
{
	mMissileSwarmTrigger += Delta;
	if (mMissileSwarmTrigger < 0.3f)
	{
		return;
	}
	mMissileSwarmTrigger = 0;

	mbMissileAiming = !mbMissileAiming;


	if (auto MissileSystem = mMissileSwarmSystem.lock())
	{
		if (!mbMissileAiming)
		{
			MissileSystem->Fire();
		}
		// 미사일 조준 준비
		else
		{
			if (!MissileSystem->TryCreateTargetMarkers(4, 5))
			{
				mbMissileAiming = false;
			}			
		}
	}
	
}
