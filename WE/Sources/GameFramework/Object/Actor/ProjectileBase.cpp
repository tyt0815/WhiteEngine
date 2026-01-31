#include "ProjectileBase.h"

AProjectileBase::AProjectileBase()
{
	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(mStaticMeshComp);
	if (auto StaticMeshComp = mStaticMeshComp.lock())
	{
		mBlueprintMap["StaticMeshComponent"] = StaticMeshComp.get();
	}

	mProjMoveComp = CreateComponent<WProjectileMovementComponent>();
	if (auto Proj = mProjMoveComp.lock())
	{
		mBlueprintMap["ProjectileMovementComponent"] = Proj.get();
	}
}
