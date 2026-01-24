#include "PhysicsComponent.h"
#include "World/World.h"

WPhysicsComponent::WPhysicsComponent() :
	mBody(std::make_unique<FPhysicsBody>(this))
{
}

void WPhysicsComponent::BeginComponent()
{
	Super::BeginComponent();

	if (!mBody->IsValid())
	{
		CreatePhysicsBody();
	}

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
		if (mBody->IsValid())
		{
			mBody->Activate();
		}
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

void WPhysicsComponent::OnSetTransform()
{
	Super::OnSetTransform();
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
		mBody->SetTransform(GetWorldTransform());
	}
}

void WPhysicsComponent::UpdateFromPhysics()
{
	if (mbPhysicSimulate && mMotionType != EMotionType::Static)
	{
		FTransform Transform = GetWorldTransform();
		Transform.Translation = mBody->GetLocation();
		Transform.Rotation = mBody->GetRotation();
		Transform.Scale = mLastPhysicsTransform.Scale;

		if (!Transform.Equal(mLastPhysicsTransform))
		{
			mLastPhysicsTransform = Transform;

			SetWorldTransform(mLastPhysicsTransform);
		}
	}
}

void WPhysicsComponent::ActivatePhysicBody()
{
	if (!mbPhysicSimulate)
	{
		mbPhysicSimulate = true;

		if (mBody->IsValid())
		{
			mBody->Activate();
		}
	}
}

void WPhysicsComponent::DeactivatePhysicBody()
{
	if (mbPhysicSimulate)
	{
		mbPhysicSimulate = false;
		if (mBody->IsValid())
		{
			mBody->Deactivate();
		}
	}
}

void WPhysicsComponent::GenerateOverlapEvent()
{
	mbGenerateOverlapEvent = true;
}

void WPhysicsComponent::SetMotionType(EMotionType MotionType)
{
	mMotionType = MotionType;
}

void WPhysicsComponent::SetObjectChannel(EObjectChannel::EObjectChannel ObjectChannel)
{
	mObjectChannel = ObjectChannel;
}

void WPhysicsComponent::CreatePhysicsBody()
{
	JPH::ShapeRefC Shape = CreatePhysicsShape();
	JPH::BodyCreationSettings Settings = JPH::BodyCreationSettings(Shape, JPH::RVec3(), JPH::Quat::sIdentity(), mMotionType, mObjectChannel);
	Settings.mIsSensor = mbGenerateOverlapEvent;
	mBody->CreateBody(Settings);
}
