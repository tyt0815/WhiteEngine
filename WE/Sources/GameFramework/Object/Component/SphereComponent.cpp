#include "SphereComponent.h"
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

using namespace JPH;

JPH::ShapeRefC WSphereComponent::CreatePhysicsShape()
{
	SphereShapeSettings SphereSettings(mRadius);
	SphereSettings.SetEmbedded();
	ShapeSettings::ShapeResult ShapeResult = SphereSettings.Create();
	return ShapeResult.Get();
}

void WSphereComponent::SetRadius(float Value)
{
	mRadius = Value;
}
