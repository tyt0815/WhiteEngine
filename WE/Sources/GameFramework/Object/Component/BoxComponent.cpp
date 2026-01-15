#include "BoxComponent.h"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

using namespace JPH;

JPH::BodyCreationSettings WBoxComponent::CreatePhysicsBodySettings()
{
	BoxShapeSettings BoxSettings(RVec3(mExtent.x, mExtent.y, mExtent.z));
	BoxSettings.SetEmbedded();
	ShapeSettings::ShapeResult ShapeResult = BoxSettings.Create();
	ShapeRefC Shape = ShapeResult.Get();

	return JPH::BodyCreationSettings(Shape, RVec3(), Quat::sIdentity(), mMotionType, mObjectChannel);
}

void WBoxComponent::SetExtent(XMFLOAT3 Extent)
{
	mExtent = Extent;
}
