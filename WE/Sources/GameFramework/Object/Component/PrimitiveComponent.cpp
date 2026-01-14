#include "PrimitiveComponent.h"
#include "GameFramework/RenderItemProxy.h"
#include "GameFramework/Object/World/World.h"

extern const int gFrameResourcesNum;

WPrimitiveComponent::WPrimitiveComponent()
{

}

void WPrimitiveComponent::BeginPlay()
{
	Super::BeginPlay();

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

void WPrimitiveComponent::UpdatePhysics()
{
	if (mbPhysicSimulate)
	{
		this->SetWorldTransform(mBody->GetTransform());
	}
}

void WPrimitiveComponent::UpdatePhysicsTransform()
{
	if (mbPhysicSimulate)
	{
		mBody->SetTransform(GetWorldTransform());
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
	MeshCbProxy->World = GetWorldMatrix();
}

void WPrimitiveComponent::ActivatePhysicBody()
{
	mBody->AddBody();
	mBody->SetTransform(GetWorldTransform());
}
