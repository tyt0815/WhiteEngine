#include "SphereComponent.h"
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

using namespace JPH;

JPH::BodyCreationSettings WSphereComponent::CreatePhysicsBodySettings()
{
	SphereShapeSettings SphereSettings(mRadius);
	SphereSettings.SetEmbedded();
	ShapeSettings::ShapeResult ShapeResult = SphereSettings.Create();
	ShapeRefC Shape = ShapeResult.Get();

	return JPH::BodyCreationSettings(Shape, RVec3(), Quat::sIdentity(), mMotionType, mObjectChannel);
}

void WSphereComponent::SetRadius(float Value)
{
	mRadius = Value;
}
