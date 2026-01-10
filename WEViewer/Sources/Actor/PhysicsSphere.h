#pragma once

#include "GameFramework/Object/Actor/Actor.h"

#include "Physics/PhysicsCore.h"

class APhysicsSphere : public AActor
{
	typedef AActor Super;
public:
	APhysicsSphere();

	virtual void Tick(float DeltaTiem) override;

private:
	std::unique_ptr<FBody> mBody;
};