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

	virtual void Tick_PrePhysics(float DeltaTiem) override;

	

private:
	void OnHit(WPrimitiveComponent* Other);

	WSphereComponent* mSphereComp;
};