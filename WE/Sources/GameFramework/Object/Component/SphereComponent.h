#pragma once
#include "PhysicsComponent.h"

class WSphereComponent : public WPhysicsComponent
{
protected:
	virtual JPH::ShapeRefC CreatePhysicsShape() override;

public:
	void SetRadius(float Value);

protected:
	float mRadius = 0.5f;
};