#include "PhysicsBody.h"
#include "JPHUtility.h"
#include "PhysicsUserData.h"

using namespace JPH;

namespace Physics
{
	 BodyInterface& GetBodyInterface();
}

FPhysicsBody::~FPhysicsBody()
{
	if (mBody != nullptr)
	{
		Physics::FUserDataManager::EnqueueRemoveQ(mUserData);
		Physics::GetBodyInterface().RemoveBody(mBody->GetID());
		Physics::GetBodyInterface().DestroyBody(mBody->GetID());
	}
}

void FPhysicsBody::Create(JPH::BodyCreationSettings Settings)
{
	UINT64 UserDataID = Physics::FUserDataManager::CreateUserData(mOwner->GetWeakPtr<WPhysicsComponent>());
	mUserData = Physics::FUserDataManager::GetUserData(UserDataID);
	Settings.mUserData = reinterpret_cast<JPH::uint64>(&mUserData->Comp);
	Settings.mPosition = ToJPHPosition(mOwner->GetWorldLocation());
	Settings.mRotation = TOJPHQuatRotation(FDXMath::EulerToQuaternion(mOwner->GetWorldRotation()));
	mBody = Physics::GetBodyInterface().CreateBody(Settings);
}

void FPhysicsBody::UpdateShape(JPH::ShapeRefC Shape)
{
	Physics::GetBodyInterface().SetShape(mBody->GetID(), Shape, false, JPH::EActivation::Activate);
}

void FPhysicsBody::Activate()
{
	BodyInterface& BI = Physics::GetBodyInterface();
	if (!BI.IsAdded(mBody->GetID()))
	{
		BI.AddBody(mBody->GetID(), EActivation::Activate);
	}
}

void FPhysicsBody::Deactivate()
{
	BodyInterface& BI = Physics::GetBodyInterface();
	if (BI.IsAdded(mBody->GetID()))
	{
		BI.RemoveBody(mBody->GetID());
	}
}

void FPhysicsBody::SetLocation(const XMFLOAT3& Location)
{
	Physics::GetBodyInterface().SetPosition(mBody->GetID(), ToJPHPosition(Location), EActivation::Activate);
}

void FPhysicsBody::SetRotation(const XMFLOAT4& Quaternion)
{
	Quat Q = TOJPHQuatRotation(Quaternion);
	Physics::GetBodyInterface().SetRotation(mBody->GetID(), Q, EActivation::Activate);
}

void FPhysicsBody::SetLocationAndRotation(const XMFLOAT3& Location, const XMFLOAT4& Quaternion)
{
	RVec3 Pos = ToJPHPosition(Location);
	Quat Rot = TOJPHQuatRotation(Quaternion);
	Physics::GetBodyInterface().SetPositionAndRotationWhenChanged(mBody->GetID(), Pos, Rot, EActivation::Activate);
}

void FPhysicsBody::SetMotiontype(EMotionType MotionType)
{
	Physics::GetBodyInterface().SetMotionType(mBody->GetID(), MotionType, JPH::EActivation::Activate);
}

void FPhysicsBody::AddImpulse(const XMFLOAT3& InImpulse, const XMFLOAT3& InPoint)
{
	Vec3Arg Impulse = ToJPHPosition(InImpulse);
	RVec3Arg Point = ToJPHPosition(InPoint);
	Physics::GetBodyInterface().AddImpulse(GetBodyID(), Impulse, Point);
}

XMFLOAT3 FPhysicsBody::GetLocation() const
{
	return ToDXLocation(Physics::GetBodyInterface().GetCenterOfMassPosition(mBody->GetID()));
}

XMFLOAT3 FPhysicsBody::GetRotation() const
{
	return FDXMath::QuaternionToEuler(ToDXQuatRotation(Physics::GetBodyInterface().GetRotation(mBody->GetID())));
}
