#include "PrimitiveComponent.h"
#include "GameFramework/RenderItemProxy.h"
#include "GameFramework/Object/World/World.h"

extern const int gFrameResourcesNum;

WPrimitiveComponent::WPrimitiveComponent()
{

}

void WPrimitiveComponent::BeginComponent()
{
	Super::BeginComponent();

	if (mBody == nullptr)
	{
		CreatePhysicsBody();
	}

	if (mBody)
	{
		mBody->mOnHitDelegate.Bind(this, &WPrimitiveComponent::OnComponentHit_Internal);
		mBody->mOnBeginOverlapDelgate.Bind(this, &WPrimitiveComponent::OnComponentBeginOverlap_Internal);
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
	if (mbPhysicSimulate)
	{
		mBody->SetTransform(GetWorldTransform());
	}
}

void WPrimitiveComponent::UpdateFromPhysics()
{
	if (mbPhysicSimulate)
	{
		this->SetWorldTransform(mBody->GetTransform());
	}
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

	if (mBody)
	{
		mBody->AddBody();
	}
}

void WPrimitiveComponent::OnComponentHit_Internal()
{
	mOnHitDelegate.Execute();
}

void WPrimitiveComponent::OnComponentBeginOverlap_Internal()
{
	mOnBeginOverlapDelegate.Execute();
}
