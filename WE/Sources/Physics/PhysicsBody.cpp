#include "PhysicsBody.h"
#include "JPHUtility.h"
#include "PhysicsUserData.h"

using namespace JPH;

namespace Physics
{
	 BodyInterface* GetBodyInterface();
}

FPhysicsBody::~FPhysicsBody()
{
	if (mBody != nullptr)
	{
		Physics::FUserDataManager::EnqueueRemoveQ(mUserData->ID);
		Physics::GetBodyInterface()->RemoveBody(mBody->GetID());
		Physics::GetBodyInterface()->DestroyBody(mBody->GetID());
	}
}

void FPhysicsBody::CreateBody(JPH::BodyCreationSettings Settings)
{
	UINT64 UserDataID = Physics::FUserDataManager::CreateUserData(mOwner->GetWeakPtr<WPhysicsComponent>());
	mUserData = Physics::FUserDataManager::GetUserData(UserDataID);
	Settings.mUserData = reinterpret_cast<JPH::uint64>(&mUserData->Comp);
	mBody = Physics::GetBodyInterface()->CreateBody(Settings);
	Physics::GetBodyInterface()->AddBody(mBody->GetID(), EActivation::Activate);
}

void FPhysicsBody::Activate()
{
	if (BodyInterface* BI = Physics::GetBodyInterface())
	{
		if (!BI->IsAdded(mBody->GetID()))
		{
			BI->AddBody(mBody->GetID(), EActivation::Activate);
		}
	}
}

void FPhysicsBody::Deactivate()
{
	if (BodyInterface* BI = Physics::GetBodyInterface())
	{
		if (BI->IsAdded(mBody->GetID()))
		{
			BI->RemoveBody(mBody->GetID());
		}
	}
}

void FPhysicsBody::SetPosition(XMFLOAT3 Position)
{
	Physics::GetBodyInterface()->SetPosition(mBody->GetID(), ToJPHPosition(Position), EActivation::Activate);
}

void FPhysicsBody::SetMotiontype(EMotionType MotionType)
{
	Physics::GetBodyInterface()->SetMotionType(mBody->GetID(), MotionType, JPH::EActivation::Activate);
}

void FPhysicsBody::SetActivate(bool bActivate)
{
	bActivate ?
		Physics::GetBodyInterface()->ActivateBody(mBody->GetID()) :
		Physics::GetBodyInterface()->DeactivateBody(mBody->GetID());
}

XMFLOAT3 FPhysicsBody::GetLocation() const
{
	return ToDXLocation(Physics::GetBodyInterface()->GetCenterOfMassPosition(mBody->GetID()));
}

XMFLOAT3 FPhysicsBody::GetRotation() const
{
	return FDXMath::QuaternionToEuler(ToDXQuatRotation(Physics::GetBodyInterface()->GetRotation(mBody->GetID())));
}

void FPhysicsBody::SetTransform(const FTransform& Transform)
{
	RVec3 Pos = ToJPHPosition(Transform.Translation);
	Quat Quat = TOJPHQuatRotation(Transform.GetQuaternionRotationFloat4());

	BodyInterface* BI = Physics::GetBodyInterface();

	JPH::EActivation Activation = BI->IsActive(mBody->GetID()) ? EActivation::Activate : EActivation::DontActivate;
	Physics::GetBodyInterface()->SetPositionAndRotation(mBody->GetID(), Pos, Quat, Activation);
}
