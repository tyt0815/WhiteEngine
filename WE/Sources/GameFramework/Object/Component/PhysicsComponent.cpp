#include "PhysicsComponent.h"
#include "World/World.h"
#include "Actor/Actor.h"


WPhysicsComponent::WPhysicsComponent() :
	mBody(std::make_unique<FPhysicsBody>(this))
{
}

void WPhysicsComponent::BeginComponent()
{
	Super::BeginComponent();

	CreatePhysicsBody();

	if (mbPhysicSimulate)
	{
		ActivatePhysicBody();
	}
}

void WPhysicsComponent::OnActivate()
{
	GetWorld()->EnqueuePhysicsComponent(this);
	if (mbPhysicSimulate)
	{
		ActivatePhysicBody();
	}
}

void WPhysicsComponent::OnDeactivate()
{
	GetWorld()->DequeuePhysicsComponent(this);
	if (mBody->IsValid())
	{
		mBody->Deactivate();
	}
}

void WPhysicsComponent::PostSetupAttachment()
{
	Super::PostSetupAttachment();
}

void WPhysicsComponent::UpdateToPhysics()
{
	FTransform WorldTransform = GetWorldTransform();
	if (mbPhysicSimulate && mMotionType != EMotionType::Static && !mLastPhysicsTransform.Equal(WorldTransform))
	{
		if (!FDXMath::Equal(mLastPhysicsTransform.Scale, WorldTransform.Scale))
		{
			mBody->UpdateShape(CreatePhysicsShape());
		}

		mLastPhysicsTransform = WorldTransform;
		mBody->SetLocation(WorldTransform.Translation);
		
		const XMFLOAT3& Loc = WorldTransform.Translation;
		XMFLOAT4 Quat = WorldTransform.GetQuaternionRotationFloat4();
		mBody->SetLocationAndRotation(Loc, Quat);
	}
}

void WPhysicsComponent::UpdateFromPhysics()
{
	if (mbPhysicSimulate && mMotionType == EMotionType::Dynamic)
	{
		FTransform Transform = GetWorldTransform();
		Transform.Translation = mBody->GetLocation();
		Transform.Rotation = mBody->GetRotation();
		Transform.Scale = mLastPhysicsTransform.Scale;

		bool bIsNan =
			std::isnan(Transform.Translation.x) || std::isnan(Transform.Translation.y) || std::isnan(Transform.Translation.z) ||
			std::isnan(Transform.Rotation.x) || std::isnan(Transform.Rotation.y) || std::isnan(Transform.Rotation.z) ||
			std::isnan(Transform.Scale.x) || std::isnan(Transform.Scale.y);

		if (!Transform.Equal(mLastPhysicsTransform) && !bIsNan)
		{
			mLastPhysicsTransform = Transform;

			SetWorldTransform(Transform);
		}
	}
}

void WPhysicsComponent::ActivatePhysicBody()
{
	mbPhysicSimulate = true;
	if (mBody->IsValid())
	{
		mBody->Activate();
	}
}

void WPhysicsComponent::DeactivatePhysicBody()
{
	mbPhysicSimulate = false;
	if (mBody->IsValid())
	{
		mBody->Deactivate();
	}
}

void WPhysicsComponent::SetMotionType(EMotionType MotionType)
{
	mMotionType = MotionType; 
	if (mBody->IsValid())
	{
		mBody->SetMotiontype(MotionType);
	}
}

void WPhysicsComponent::SetObjectChannel(EObjectChannel::EObjectChannel ObjectChannel)
{
	mObjectChannel = ObjectChannel;
	if (mBody->IsValid())
	{
		SetObjectChannel(mObjectChannel);
	}
}

namespace Physics
{
	extern class JPH::GroupFilter* g_GroupFilter;
}

void WPhysicsComponent::CreatePhysicsBody()
{
	JPH::ShapeRefC Shape = CreatePhysicsShape();
	JPH::BodyCreationSettings Settings = JPH::BodyCreationSettings(Shape, JPH::RVec3(), JPH::Quat::sIdentity(), mMotionType, mObjectChannel);
	Settings.mIsSensor = mbGenerateOverlapEvent;
	UINT GroupID = GetOwner().lock()->mActorCounter;
	Settings.mCollisionGroup.SetGroupID(GroupID);
	Settings.mCollisionGroup.SetGroupFilter(Physics::g_GroupFilter);
	mBody->Create(Settings);
}
