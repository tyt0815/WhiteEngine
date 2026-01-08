#pragma once
#include "MovementComponent.h"

class WProjectileMovementComponent : public WMovementComponent
{
	typedef WMovementComponent Super;
public:
	virtual void TickComponent(float DeltaTime) override;

protected:
	float mLiftSpan = 5.0f;

private:
	float mLifeTimeElapsed = 0.0f;
};