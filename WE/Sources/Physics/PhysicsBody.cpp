#include "PhysicsBody.h"
#include "JPHUtility.h"

using namespace JPH;

namespace Physics
{
	 BodyInterface* GetBodyInterface();
}

FPhysicsBody::~FPhysicsBody()
{
	RemoveBody();
	Physics::GetBodyInterface()->DestroyBody(mBody->GetID());
}

void FPhysicsBody::CreateBody(JPH::BodyCreationSettings Settings)
{
	mBody = Physics::GetBodyInterface()->CreateBody(Settings);
}

void FPhysicsBody::AddBody(bool bActivate)
{
	Physics::GetBodyInterface()->AddBody(mBody->GetID(), bActivate ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
}

void FPhysicsBody::RemoveBody()
{
	Physics::GetBodyInterface()->RemoveBody(mBody->GetID());
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

FTransform FPhysicsBody::GetTransform() const
{
	RVec3 Location = Physics::GetBodyInterface()->GetCenterOfMassPosition(mBody->GetID());
	JPH::Quat QuatRotation = Physics::GetBodyInterface()->GetRotation(mBody->GetID());

	FTransform Transform = FTransform::Default;
	Transform.Translation = ToDXLocation(Location);
	Transform.SetRotationByQuat(ToDXQuatRotation(QuatRotation));

	return Transform;
}

void FPhysicsBody::SetTransform(const FTransform& Transform)
{
	RVec3 Pos = ToJPHPosition(Transform.Translation);
	Quat Quat = TOJPHQuatRotation(Transform.GetQuaternionRotationFloat4());

	BodyInterface* BI = Physics::GetBodyInterface();

	JPH::EActivation Activation = BI->IsActive(mBody->GetID()) ? EActivation::Activate : EActivation::DontActivate;
	Physics::GetBodyInterface()->SetPositionAndRotation(mBody->GetID(), Pos, Quat, Activation);
}
