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
		ProjMoveComp->SetLifeSpan(10.0f);
		ProjMoveComp->SetHoming(true);
		ProjMoveComp->SetHomingTurnLimit(45);
	}

	mTickGroup = ETickGroup::ETG_PostPhysics;
}

void ATopAttackMissile::SetTargetPosition(XMFLOAT3 Pos)
{
	if (mTargetMarker.expired())
	{
		mTargetMarker = GetWorld()->SpawnActor<AActor>();
	}

	if (auto Marker = mTargetMarker.lock())
	{
		Marker->SetActorLocation(Pos);

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

void ATopAttackMissile::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	XMFLOAT3 Start = GetActorLocation();
	XMFLOAT3 Forward = GetFowardVector();
	XMFLOAT3 End = Start;
	Start.x += Forward.x;
	Start.y += Forward.y;
	Start.z += Forward.z;
	GetWorld()->DrawDebugLine(Start, End, XMFLOAT4(0, 1, 1, 1), 0);
}

