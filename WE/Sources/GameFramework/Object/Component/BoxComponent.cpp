#include "BoxComponent.h"

void WBoxComponent::CreatePhysicsBody()
{
	mBody = CreateBoxBody(mExtent, mObjectType);
}
