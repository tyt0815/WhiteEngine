#include "BoxComponent.h"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

using namespace JPH;

JPH::ShapeRefC WBoxComponent::CreatePhysicsShape()
{
	XMFLOAT3 ScaledExtend = GetScaledExtent();
	BoxShapeSettings BoxSettings(RVec3(ScaledExtend.x, ScaledExtend.y, ScaledExtend.z));
	BoxSettings.SetEmbedded();
	ShapeSettings::ShapeResult ShapeResult = BoxSettings.Create();
	return ShapeResult.Get();
}

void WBoxComponent::SetExtent(XMFLOAT3 Extent)
{
	mExtent = Extent;
}

XMFLOAT3 WBoxComponent::GetScaledExtent()
{
	XMFLOAT3 Scale= GetWorldTransform().Scale;
	XMStoreFloat3(&Scale, XMVectorMultiply(XMLoadFloat3(&Scale), XMLoadFloat3(&mExtent)));
	return Scale;
}
