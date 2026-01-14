#pragma once
#include "PrimitiveComponent.h"

class WSphereComponent : public WPrimitiveComponent
{
protected:
	void CreatePhysicsBody() override;

protected:
	float mRadius = 1;
};