#include "BoxComponent.h"

void WBoxComponent::CreatePhysicsBody()
{
	mBody = CreateBoxBody(mExtent, mObjectType);
}

void WBoxComponent::SetExtent(XMFLOAT3 Extent)
{
	mExtent = Extent;
}
