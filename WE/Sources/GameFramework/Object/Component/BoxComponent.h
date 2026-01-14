#pragma once
#include "PrimitiveComponent.h"

class WBoxComponent : public WPrimitiveComponent
{
protected:
	void CreatePhysicsBody() override;

protected:
	XMFLOAT3 mExtent = { 1.0f, 1.0f, 1.0f };
};