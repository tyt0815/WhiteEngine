#include "SphereComponent.h"

void WSphereComponent::CreatePhysicsBody()
{
	mBody = CreateSphereBody(mRadius, mObjectType);
}

void WSphereComponent::SetRadius(float Value)
{
	mRadius = Value;
}
