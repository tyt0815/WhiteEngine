#include "PrimitiveComponent.h"
#include "GameFramework/RenderItemProxy.h"
#include "GameFramework/Object/World/World.h"

void WPrimitiveComponent::Update()
{
	Super::Update();

	UpdateConstantBufferIndex();

	UpdateProxies();
}

void WPrimitiveComponent::UpdateConstantBufferIndex()
{
	mMeshCBIndex = GetWorld()->AllocateMeshCbProxy();
}

void WPrimitiveComponent::UpdateProxies()
{
	FMeshCBProxy* MeshCbProxy = GetWorld()->GetMeshCBProxy(mMeshCBIndex);
	MeshCbProxy->World = GetWorldFloat4x4();
}