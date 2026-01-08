#include "StaticMeshComponent.h"
#include "Render/MeshGeometry.h"
#include "GameFramework/Object/World/World.h"

extern const int gFrameResourcesNum;

WStaticMeshComponent::WStaticMeshComponent()
{
}

void WStaticMeshComponent::UpdateConstantBufferIndex()
{
	Super::UpdateConstantBufferIndex();

	mStaticMeshProxyIndecies.clear();
	mSubmeshCBIndices.clear();
	WWorld* World = GetWorld();
	for (int i = 0; i < mStaticMesh.Geometry->DrawArgs.size(); ++i)
	{
		mStaticMeshProxyIndecies.push_back(World->AllocateStaticMeshCbProxy());
		mSubmeshCBIndices.push_back(World->AllocateSubmeshCbProxy());
	}
}

void WStaticMeshComponent::UpdateProxies()
{
	Super::UpdateProxies();

	WWorld* World = GetWorld();
	for (int i = 0; i < mStaticMesh.Geometry->DrawArgs.size(); ++i)
	{
		FStaticMeshProxy* StaticMeshProxy = World->GetStaticMeshProxy(mStaticMeshProxyIndecies[i]);
		StaticMeshProxy->MeshGeometry = mStaticMesh.Geometry;
		StaticMeshProxy->Material = mStaticMesh.Material;
		StaticMeshProxy->MeshCBIndex = mMeshCBIndex;
		StaticMeshProxy->SubmeshCBIndex = mSubmeshCBIndices[i];
		StaticMeshProxy->IndexCount = StaticMeshProxy->MeshGeometry->DrawArgs[i].IndexCount;
		StaticMeshProxy->StartIndexLocation = StaticMeshProxy->MeshGeometry->DrawArgs[i].StartIndexLocation;
		StaticMeshProxy->BaseVertexLocation = StaticMeshProxy->MeshGeometry->DrawArgs[i].BaseVertexLocation;
		StaticMeshProxy->bCastShadow = mbCastShadow;

		FSubmeshCBProxy* SubmeshCBProxy = World->GetSubmeshCBProxy(mSubmeshCBIndices[i]);
		SubmeshCBProxy->MaterialIndex = mStaticMesh.Material->Type;
	}
}

void WStaticMeshComponent::SetStaticMesh(const FStaticMesh& StaticMesh)
{
	mStaticMesh = StaticMesh;
}