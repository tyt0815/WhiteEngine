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

	CreatePhysicsBody();

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
	mBody->AddBody();
	mBody->SetTransform(GetWorldTransform());
}
