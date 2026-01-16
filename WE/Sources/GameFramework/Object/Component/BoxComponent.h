#pragma once
#include "PhysicsComponent.h"

class WBoxComponent : public WPhysicsComponent
{
protected:
	virtual JPH::BodyCreationSettings CreatePhysicsBodySettings() override;

public:
	void SetExtent(XMFLOAT3 Extent);

protected:
	XMFLOAT3 mExtent = { 1.0f, 1.0f, 1.0f };
};