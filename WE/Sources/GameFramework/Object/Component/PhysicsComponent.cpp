#include "PhysicsComponent.h"

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
	mbPhysicSimulate = true;

	if (mBody->IsValid())
	{
		mBody->AddBody();
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
