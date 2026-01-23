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

void WPhysicsComponent::UpdateToPhysics()
{
	if (mbPhysicSimulate && mMotionType != EMotionType::Static)
	{
		mBody->SetTransform(GetWorldTransform());
	}
}

void WPhysicsComponent::UpdateFromPhysics()
{
	if (mbPhysicSimulate && mMotionType != EMotionType::Static)
	{
		this->SetWorldTransform(mBody->GetTransform());
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
	auto Settings = CreatePhysicsBodySettings();
	Settings.mIsSensor = mbGenerateOverlapEvent;
	mBody->CreateBody(Settings);
}
