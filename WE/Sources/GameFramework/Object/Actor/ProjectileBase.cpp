#include "ProjectileBase.h"

AProjectileBase::AProjectileBase()
{
	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	
	if (auto StaticMeshComp = mStaticMeshComp.lock())
	{
		StaticMeshComp->SetupAttachment(GetRootComponent());
		RegisterWProperty("StaticMeshComponent", StaticMeshComp.get());
	}

	mProjMoveComp = CreateComponent<WProjectileMovementComponent>();
	if (auto Proj = mProjMoveComp.lock())
	{
		RegisterWProperty("ProjMovementComp", Proj.get());
	}

	mObjAnimComp = CreateComponent<WObjectAnimComponent>();
	if (auto AnimComp = mObjAnimComp.lock())
	{
		RegisterWProperty("ObjectAnimComp", AnimComp.get());
		AnimComp->LoadKeyframesFromOADAsset(L"OA_MissileTrack", "Proj3");
	}

	RegisterWProperty("Value", &Value);
	RegisterWProperty("Boolean", &Boolean);
}

void AProjectileBase::PlayAnimation()
{
	mObjAnimComp.lock()->Play(true, ERootMotion::All);
}