#include "PrimitiveComponent.h"
#include "GameFramework/RenderItemProxy.h"
#include "GameFramework/Object/World/World.h"

extern const int gFrameResourcesNum;

WPrimitiveComponent::WPrimitiveComponent():
	mBody(std::make_unique<FPhysicsBody>(this))
{

}

void WPrimitiveComponent::BeginComponent()
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

void WPrimitiveComponent::Update()
{
	Super::Update();

	UpdateConstantBufferIndex();

	UpdateProxies();
}

void WPrimitiveComponent::UpdateToPhysics()
{
	if (mbPhysicSimulate && mMotionType != EMotionType::Static)
	{
		mBody->SetTransform(GetWorldTransform());
	}
}

void WPrimitiveComponent::UpdateFromPhysics()
{
	if (mbPhysicSimulate && mMotionType != EMotionType::Static)
	{
		this->SetWorldTransform(mBody->GetTransform());
	}
}

void WPrimitiveComponent::CreatePhysicsBody()
{
	auto Settings = CreatePhysicsBodySettings();
	Settings.mUserData = reinterpret_cast<JPH::uint64>(this);
	Settings.mIsSensor = mbGenerateOverlapEvent;
	mBody->CreateBody(Settings);
}

void WPrimitiveComponent::UpdateConstantBufferIndex()
{
	WWorld* World = GetWorld();
	mMeshCBIndex = GetWorld()->AllocateMeshCbProxy();
}

void WPrimitiveComponent::UpdateProxies()
{
	FMeshCBProxy* MeshCbProxy = GetWorld()->GetMeshCBProxy(mMeshCBIndex);
	MeshCbProxy->World = GetWorldFloat4x4();
}

void WPrimitiveComponent::ActivatePhysicBody()
{
	mbPhysicSimulate = true;

	if (mBody->IsValid())
	{
		mBody->AddBody();
	}
}

void WPrimitiveComponent::SetMotionType(EMotionType MotionType)
{
	mMotionType = MotionType;
}

void WPrimitiveComponent::SetObjectChannel(EObjectChannel::EObjectChannel ObjectChannel)
{
	mObjectChannel = ObjectChannel;
}

void WPrimitiveComponent::GenerateOverlapEvent()
{
	mbGenerateOverlapEvent = true;
}
