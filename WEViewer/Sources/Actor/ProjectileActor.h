#pragma once

#include "GameFramework/Object/Actor/Actor.h"

class WProjectileMovementComponent;

class AProjectileActor : public AActor
{
	typedef AActor Super;
public:
	AProjectileActor();

public:
	void SetLifeSpan(float LifeSpan);

protected:
	WProjectileMovementComponent* ProjComp;
};