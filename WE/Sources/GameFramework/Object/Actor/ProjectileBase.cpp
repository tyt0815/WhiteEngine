#include "ProjectileBase.h"

AProjectileBase::AProjectileBase()
{
	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	
	if (auto StaticMeshComp = mStaticMeshComp.lock())
	{
		StaticMeshComp->SetupAttachment(GetRootComponent());
		StaticMeshComp->SetStaticMesh("SM_LaminateFlooringBrownBox");
		RegisterWProperty("StaticMeshComponent", StaticMeshComp.get());
	}

	mProjMoveComp = CreateComponent<WProjectileMovementComponent>();
	if (auto Proj = mProjMoveComp.lock())
	{
		Proj->mVelocity = XMFLOAT3(0, 0, 1);
		Proj->SetHoming(true);
		Proj->SetLifeSpan(10.0f);
		Proj->SetHomingTurnLimit(10);
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

void AProjectileBase::SetHomingTarget(TWeakPtr<WSceneComponent> Target)
{
	if (auto Proj = mProjMoveComp.lock())
	{
		Proj->SetHomingTarget(Target);
	}
}