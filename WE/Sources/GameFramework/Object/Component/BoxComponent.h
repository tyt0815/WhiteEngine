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
	XMFLOAT3 mExtent = { 0.5f, 0.5f, 0.5f };
};