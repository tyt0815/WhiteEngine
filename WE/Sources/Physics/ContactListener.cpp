#include "ContactListener.h"
#include "World/World.h"
#include "JPHUtility.h"

using namespace JPH;

ValidateResult MyContactListener::OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult)
{
	return ValidateResult::AcceptAllContactsForThisBodyPair;
}

void MyContactListener::OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
	FPhysicEventInfo Info;
	Info.Comp1 = *reinterpret_cast<TWeakPtr<WPhysicsComponent>*>(inBody1.GetUserData());
	Info.Comp2 = *reinterpret_cast<TWeakPtr<WPhysicsComponent>*>(inBody2.GetUserData());
	Info.ImpactPoint1 = ToDXLocation(inManifold.GetWorldSpaceContactPointOn1(0));
	Info.ImpactPoint2 = ToDXLocation(inManifold.GetWorldSpaceContactPointOn2(0));

	if (Info.Comp1.expired() || Info.Comp2.expired())
	{
		return;
	}
	WWorld* World = Info.Comp1.lock()->GetWorld();

	if (inBody1.IsSensor() || inBody2.IsSensor())
	{
		World->EnqueueOnBeginOverlapEvent(Info);
	}
	else
	{
		World->EnqueueOnHitEvent(Info);
	}
}

void MyContactListener::OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
}

void MyContactListener::OnContactRemoved(const SubShapeIDPair& inSubShapePair)
{
}
