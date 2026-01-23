#pragma once

#include "GameFramework/Object/Actor/Actor.h"

#include "Physics/PhysicsCore.h"

class WSphereComponent;

class APhysicsSphere : public AActor
{
	typedef AActor Super;
public:
	APhysicsSphere();

	virtual void BeginPlay() override;

	

private:
	void OnHit(TWeakPtr<WPhysicsComponent> Other, XMFLOAT3 ImpactPoint);

	TWeakPtr<WSphereComponent> mSphereComp;
};