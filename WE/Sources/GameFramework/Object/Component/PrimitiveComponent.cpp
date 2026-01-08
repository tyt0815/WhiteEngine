#include "PrimitiveComponent.h"
#include "GameFramework/RenderItemProxy.h"
#include "GameFramework/Object/World/World.h"

extern const int gFrameResourcesNum;

WPrimitiveComponent::WPrimitiveComponent()
{

}

void WPrimitiveComponent::Update()
{
	Super::Update();

	UpdateConstantBufferIndex();

	UpdateProxies();
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
