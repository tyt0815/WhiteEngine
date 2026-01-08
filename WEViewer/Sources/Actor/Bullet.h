#pragma once
#include "ProjectileActor.h"

class WStaticMeshComponent;

class ABullet : public AProjectileActor
{
public:
	ABullet();

private:
	WStaticMeshComponent* StaticMeshComp;
};