#pragma once
#include "Actor.h"
#include "Component/BoxComponent.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/ObjectAnimComponent.h"

class AProjectileBase : public AActor
{
public:
	AProjectileBase();

private:
	void SmartSetHomingTarget();

};

REGISTER_ACTOR(AProjectileBase);