#pragma once
#include "SceneComponent.h"

class WMovementComponent : public WSceneComponent
{
	typedef WSceneComponent Super;
public:
	WMovementComponent();

	virtual void Tick(float DeltaTime) override;

public:
	XMFLOAT3 mVelocity = { 0, 0, 0 };

	XMFLOAT3 mAcceleration = {0, 0, 0};
};