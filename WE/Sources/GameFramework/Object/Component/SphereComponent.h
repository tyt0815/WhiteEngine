#pragma once
#include "PrimitiveComponent.h"

class WSphereComponent : public WPrimitiveComponent
{
protected:
	virtual JPH::BodyCreationSettings CreatePhysicsBodySettings() override;

public:
	void SetRadius(float Value);

protected:
	float mRadius = 0.5f;
};