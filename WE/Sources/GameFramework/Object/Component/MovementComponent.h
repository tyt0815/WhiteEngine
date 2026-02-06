#pragma once
#include "ActorComponent.h"

class WMovementComponent : public WActorComponent
{
	typedef WActorComponent Super;
public:
	WMovementComponent();

	virtual void Tick(float DeltaTime) override;

public:
	XMFLOAT3 mVelocity = { 0, 0, 0 };

	XMFLOAT3 mAcceleration = {0, 0, 0};

private:
	void MoveOwner(XMFLOAT3 WorldDirection);
};