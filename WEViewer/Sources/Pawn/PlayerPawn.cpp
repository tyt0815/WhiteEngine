#include "PlayerPawn.h"
#include "World/World.h"
#include "Interface/InteractionInterface.h"

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

	GetInputSystemManager()->BindKeyboardAction('1', EKeyboardInputType::EKIT_Down, this, &APlayerPawn::FireArcProjectile);
	GetInputSystemManager()->BindKeyboardAction('2', EKeyboardInputType::EKIT_Pressed, this, &APlayerPawn::TriggerMissileSwarm);
	GetInputSystemManager()->BindKeyboardAction('f', EKeyboardInputType::EKIT_Pressed, this, &APlayerPawn::Interaction);
}

void APlayerPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	mArcProjectileCoolTime = max(mArcProjectileCoolTime - Delta, 0);

	WWorld* World = GetWorld();
	XMFLOAT3 CurrLoc = GetActorLocation();
	const XMVECTOR vCurrLoc = XMLoadFloat3(&CurrLoc);
	XMFLOAT3 Forward = GetForwardVector();
	const XMVECTOR ForwardV = XMLoadFloat3(&Forward);
	XMFLOAT3 Right = GetRightVector();
	XMFLOAT3 CurrRot = GetActorRotation();

	// Interaction
	{
		const XMFLOAT3& TraceStart = CurrLoc;
		XMFLOAT3 TraceEnd;
		XMStoreFloat3(&TraceEnd, XMVectorAdd(vCurrLoc, XMVectorScale(ForwardV, 10.0f)));

		TArray<AActor*> ActorsToIgnore;
		FHitResult HitResult;
		World->LineTrace(
			TraceStart, TraceEnd,
			ActorsToIgnore,
			HitResult,
			true,
			0
		);

		auto HittedActor = HitResult.Actor.lock();
		IInteractionInterface* Target = dynamic_cast<IInteractionInterface*>(HittedActor.get());

		if (mInteractionTarget != Target)
		{
			if (mInteractionTarget)
			{
				mInteractionTarget->OnEndInteractionFocus();
			}

			if (Target)
			{
				Target->OnBeginInteractionFocus();
			}
			mInteractionTarget = Target;
		}
	}

	if (mbMissileAiming)
	{
		const XMFLOAT3& TraceStart = CurrLoc;
		XMFLOAT3 TraceEnd;
		XMStoreFloat3(&TraceEnd, XMVectorAdd(vCurrLoc, XMVectorScale(ForwardV, 100.0f)));

		TArray<AActor*> ActorsToIgnore;
		FHitResult HitResult;
		World->BoxTrace(
			TraceStart, TraceEnd,
			XMFLOAT3(0, 1.0f, 1.0f),
			CurrRot,
			ActorsToIgnore,
			HitResult,
			false,
			0
		);

		mMissileSwarmSystem->SetActorTransform(GetActorTransform());
		mMissileSwarmSystem->SetTargetMarkersLocation(HitResult.ImpactPoint, Right, 1);
	}
}

void APlayerPawn::TriggerMissileSwarm(float Delta)
{
	mbMissileAiming = !mbMissileAiming;

	if (auto MissileSystem = mMissileSwarmSystem)
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

void APlayerPawn::FireArcProjectile(float Delta)
{
	if (mArcProjectileCoolTime > 0)
	{
		return;
	}
	mArcProjectileCoolTime = mArcProjectileDelay;

	FActorSpawnParameter Param;
	Param.Transform = GetActorTransform();
	Param.Transform.Scale = XMFLOAT3(1, 1, 1);

	GetWorld()->SpawnActorByFactory<AActor>("BP_ArcProjectile", Param);
}

void APlayerPawn::Interaction(float Delta)
{
	if (mInteractionTarget)
	{
		mInteractionTarget->Interaction();
	}
	else
	{
		std::cout << "No interaction target" << std::endl;
	}
}
