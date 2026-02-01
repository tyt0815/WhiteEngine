#pragma once
#include "Actor.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/ObjectAnimComponent.h"

class AProjectileBase : public AActor
{
public:
	AProjectileBase();

public:
	void PlayAnimation();

	void SetHomingTarget(TWeakPtr<WSceneComponent> Target);

private:
	TWeakPtr<WStaticMeshComponent> mStaticMeshComp;
	TWeakPtr<WProjectileMovementComponent> mProjMoveComp;
	TWeakPtr<WObjectAnimComponent> mObjAnimComp;

	float Value;
	bool Boolean;
};

REGISTER_ACTOR(AProjectileBase);