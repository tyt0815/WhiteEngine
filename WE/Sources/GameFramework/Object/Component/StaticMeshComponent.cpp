#include "StaticMeshComponent.h"
#include "Render/MeshGeometry.h"
#include "GameFramework/Object/World/World.h"

extern const int gFrameResourcesNum;

using namespace JPH;

WStaticMeshComponent::WStaticMeshComponent()
{
}

void WStaticMeshComponent::UpdateConstantBufferIndex()
{
	Super::UpdateConstantBufferIndex();

	if (mStaticMesh.Geometry == nullptr)
	{
		return;
	}
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

	if (mStaticMesh.Geometry == nullptr)
	{
		return;
	}

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

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
JPH::BodyCreationSettings WStaticMeshComponent::CreatePhysicsBodySettings()
{
	// TODO: 임시로 Box 형태로 만듦
	BoxShapeSettings BoxSettings(RVec3(1, 1, 1));
	BoxSettings.SetEmbedded();
	ShapeSettings::ShapeResult ShapeResult = BoxSettings.Create();
	ShapeRefC Shape = ShapeResult.Get();

	return JPH::BodyCreationSettings(Shape, RVec3(), Quat::sIdentity(), mMotionType, mObjectChannel);
}

void WStaticMeshComponent::SetStaticMesh(const FStaticMesh& StaticMesh)
{
	mStaticMesh = StaticMesh;
}