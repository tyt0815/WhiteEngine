#pragma once
#include "PrimitiveComponent.h"

class WBoxComponent : public WPrimitiveComponent
{
protected:
	virtual JPH::BodyCreationSettings CreatePhysicsBodySettings() override;

public:
	void SetExtent(XMFLOAT3 Extent);

protected:
	XMFLOAT3 mExtent = { 1.0f, 1.0f, 1.0f };
};