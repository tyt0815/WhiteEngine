#pragma once

#include "Actor/Actor.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/StaticMeshComponent.h"

class ATopAttackMissile : public AActor
{
public:
	ATopAttackMissile();

private:
	TWeakPtr<WProjectileMovementComponent> mProjectileMovementComponent;

	TWeakPtr<WStaticMeshComponent> mStaticMesh;
};