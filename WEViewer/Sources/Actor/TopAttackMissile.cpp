#include "TopAttackMissile.h"
#include "World/World.h"
#include "MissileGridManager.h"
#include "Interface/HitInterface.h"
#include "Component/ProjectileMovementComponent.h"

ATopAttackMissile::ATopAttackMissile()
{
	SetTickGroup(ETickGroup::ETG_PostPhysics, ETickPriority::ETP_High);
}

void ATopAttackMissile::OnDestroy()
{
	if (mMissileGridManager)
	{
		mMissileGridManager->RemoveMissile(this);
	}

	Super::OnDestroy();
}

void ATopAttackMissile::Initialize(WSceneComponent* HomingTarget)
{
	if (WProjectileMovementComponent* ProjComp = GetComponent<WProjectileMovementComponent>())
	{
		ProjComp->SetHomingTarget(HomingTarget);
	}
}

void ATopAttackMissile::OnHit(const FHitResult& Hit)
{
	XMVECTOR ImpactPointV = XMLoadFloat3(&Hit.ImpactPoint);
	XMVECTOR ImpactPointNormalV = XMLoadFloat3(&Hit.Normal);
	XMFLOAT3 ImpactPointWithOffset;		// ImpactPoint보다 Normal방향으로 offset을 더한 값
	XMStoreFloat3(
		&ImpactPointWithOffset,
		XMVectorMultiplyAdd(ImpactPointNormalV, XMVectorReplicate(0.1f), ImpactPointV)
	);

	if (mMissileGridManager)
	{
		mMissileGridManager->RemoveMissile(this);
	}

	Destroy();
}