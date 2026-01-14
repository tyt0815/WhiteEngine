#include "SphereComponent.h"

void WSphereComponent::CreatePhysicsBody()
{
	mBody = CreateSphereBody(mRadius, mObjectType);
}
