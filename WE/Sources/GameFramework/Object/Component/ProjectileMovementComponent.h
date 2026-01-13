#pragma once
#include "MovementComponent.h"

class WProjectileMovementComponent : public WMovementComponent
{
	typedef WMovementComponent Super;
public:
	virtual void TickComponent_PrePhysics(float DeltaTime) override;

protected:
	// 투사체 수명. 0일시 영구 지속
	float mLifeSpan = 5.0f;

private:
	float mLifeTimeElapsed = 0.0f;

public:
	void SetLifeSpan(float LifeSpan)
	{
		mLifeSpan = LifeSpan;
	}
};