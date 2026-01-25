#include "TopAttackMissile.h"
#include "World/World.h"

ATopAttackMissile::ATopAttackMissile()
{
	mStaticMesh = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(mStaticMesh);
	if (auto StaticMeshComp = mStaticMesh.lock())
	{
		StaticMeshComp->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(EStaticMeshType::ESMT_MetalCylinder));
	}

	mProjectileMovementComponent = CreateComponent<WProjectileMovementComponent>();
	if (auto ProjMoveComp = mProjectileMovementComponent.lock())
	{
		ProjMoveComp->mVelocity = XMFLOAT3(0, 0, 5);
		ProjMoveComp->SetLifeSpan(2.0f);
		ProjMoveComp->SetHoming(true);
		ProjMoveComp->SetHomingTurnLimit(360);
	}
}

void ATopAttackMissile::SetTargetLocation(XMFLOAT3 Loc)
{
	if (mTargetMarker.expired())
	{
		mTargetMarker = GetWorld()->SpawnActor<AActor>();
	}

	if (auto Marker = mTargetMarker.lock())
	{
		Marker->SetActorLocation(Loc);

		if (auto ProjMoveComp = mProjectileMovementComponent.lock())
		{
			ProjMoveComp->SetHomingTarget(Marker->GetRootComponent());
		}		
	}
}

void ATopAttackMissile::OnDestroy()
{
	if (auto Marker = mTargetMarker.lock())
	{
		Marker->Destroy();
	}
	Super::OnDestroy();
}

