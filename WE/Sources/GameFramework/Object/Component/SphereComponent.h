#pragma once
#include "PhysicsComponent.h"

class WSphereComponent : public WPhysicsComponent
{
protected:
	virtual JPH::BodyCreationSettings CreatePhysicsBodySettings() override;

public:
	void SetRadius(float Value);

protected:
	float mRadius = 0.5f;
};