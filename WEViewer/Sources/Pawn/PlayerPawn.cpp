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

	GetInputSystemManager()->BindKeyboardAction('1', this, &APlayerPawn::MissileSwarm);
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

			FTransform Transform;
			Transform.Translation = HitResult.ImpactPoint;
			XMVECTOR WorldUpV = XMVectorSet(0, 1, 0, 0);
			XMVECTOR MarkerForwardV = XMVector3Cross(RightV, WorldUpV);
			XMVECTOR MarkerRightV = XMVector3Cross(WorldUpV, MarkerForwardV);
			Transform.Rotation = FDXMath::GetEulerRotationFromVectors(MarkerForwardV, MarkerRightV, WorldUpV);
			MissileSystem->SetTargetMarkerTransform(Transform);
		}
	}	
}

void APlayerPawn::MissileSwarm(float Delta)
{
	mbMissileAiming = !mbMissileAiming;

	// TODO: 미사일 발사 로직
	if (!mbMissileAiming)
	{

	}
	// 미사일 조준 준비
	else
	{
		if (auto MissileSystem = mMissileSwarmSystem.lock())
		{
			MissileSystem->CreateTargetMarkers(4, 5, 1);
		}
	}
}
