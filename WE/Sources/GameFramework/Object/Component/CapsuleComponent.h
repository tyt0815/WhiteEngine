#pragma once

#include "PhysicsComponent.h"

class WCapsuleComponent : public WPhysicsComponent
{
protected:
	virtual JPH::ShapeRefC CreatePhysicsShape() override;

public:
	float GetScaledRadius();

	float GetScaledHalfHeight();

protected:
	float mRadius = 0.5f;
	float mHalfHeight = 0.5f;

public:
	__forceinline void SetRadius(float Value)
	{
		mRadius = Value;
	}

	__forceinline void SetHalfHeight(float Value)
	{
		mHalfHeight = Value;
	}
};