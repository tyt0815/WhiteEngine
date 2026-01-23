#pragma once
#include "PhysicsComponent.h"

class WBoxComponent : public WPhysicsComponent
{
protected:
	virtual JPH::ShapeRefC CreatePhysicsShape() override;

public:
	void SetExtent(XMFLOAT3 Extent);

	XMFLOAT3 GetScaledExtent();

protected:
	XMFLOAT3 mExtent = { 1.0f, 1.0f, 1.0f };
};