#pragma once
#include "Actor.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/StaticMeshComponent.h"

class AProjectileBase : public AActor
{
public:
	AProjectileBase();

private:
	TWeakPtr<WStaticMeshComponent> mStaticMeshComp;
	TWeakPtr<WProjectileMovementComponent> mProjMoveComp;

	float Value;
	bool Boolean;
};

REGISTER_ACTOR(AProjectileBase);