#include "ContactListener.h"
#include "World/World.h"
#include "JPHUtility.h"

using namespace JPH;

namespace Physics
{
    BodyInterface& GetBodyInterfaceNoLock();
}

using namespace Physics;

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
    BodyInterface& BI = GetBodyInterfaceNoLock();

    if (!BI.IsAdded(inSubShapePair.GetBody1ID()) || !BI.IsAdded(inSubShapePair.GetBody2ID()))
    {
        return;
    }

    FPhysicEventInfo Info;
    Info.Comp1 = *reinterpret_cast<TWeakPtr<WPhysicsComponent>*>(BI.GetUserData(inSubShapePair.GetBody1ID()));
    Info.Comp2 = *reinterpret_cast<TWeakPtr<WPhysicsComponent>*>(BI.GetUserData(inSubShapePair.GetBody2ID()));
    
    bool bAnySensor = BI.IsSensor(inSubShapePair.GetBody1ID()) || BI.IsSensor(inSubShapePair.GetBody2ID());

    // --- 안전 장치 추가 ---
    auto LockedComp1 = Info.Comp1.lock();
    auto LockedComp2 = Info.Comp2.lock();

    // 둘 중 하나라도 없으면 이벤트를 처리할 대상이 없는 것이므로 리턴
    if (!LockedComp1 || !LockedComp2)
    {
        return;
    }

    WWorld* World = LockedComp1->GetWorld();
    if (!World) return;

    if (bAnySensor)
    {
        World->EnqueueOnEndOverlapEvent(Info);
    }
    else
    {
        World->EnqueueOnExitHitEvent(Info);
    }
}
