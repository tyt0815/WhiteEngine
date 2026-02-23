#pragma once
#include "Actor/Actor.h"
#include "Component/BoxComponent.h"

class APhysicsBox : public AActor
{
	typedef AActor Super;

public:
	APhysicsBox();

	void AddImpulse(const XMFLOAT3& Impulse, const XMFLOAT3& Point);

private:
	WBoxComponent* mBoxComp;
};