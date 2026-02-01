#include "ProjectileBase.h"

AProjectileBase::AProjectileBase()
{
	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(mStaticMeshComp);
	if (auto StaticMeshComp = mStaticMeshComp.lock())
	{
		RegisterWProperty("StaticMeshComponent", StaticMeshComp.get());
	}

	mProjMoveComp = CreateComponent<WProjectileMovementComponent>();
	if (auto Proj = mProjMoveComp.lock())
	{
		RegisterWProperty("ProjMovementComp", Proj.get());
	}

	RegisterWProperty("Value", &Value);
	RegisterWProperty("Boolean", &Boolean);
}
