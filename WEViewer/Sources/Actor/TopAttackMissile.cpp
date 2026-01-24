#include "TopAttackMissile.h"

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
		ProjMoveComp->mVelocity = XMFLOAT3(0, 0, 1);
		ProjMoveComp->SetLifeSpan(5.0f);
	}
}

