#pragma once

#include "GameFramework/Object/Actor/Actor.h"

#include "Physics/PhysicsCore.h"

class WSphereComponent;

class APhysicsSphere : public AActor
{
	typedef AActor Super;
public:
	APhysicsSphere();	

private:
	TWeakPtr<WSphereComponent> mSphereComp;
};