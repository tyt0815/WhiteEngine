#include "StaticMeshComponent.h"
#include "Render/MeshGeometry.h"
#include "Render/RenderItemManager.h"

WStaticMeshComponent::WStaticMeshComponent()
{
}

void WStaticMeshComponent::SetStaticMesh(const FStaticMesh& StaticMesh)
{
	for (size_t Id : mDrawArgsPoolIds)
	{
		GetRenderItemManager()->RemoveDrawArgs(Id);
	}
	mStaticMesh = StaticMesh;
	FPrimitiveDrawArguments DrawArgs;
	DrawArgs.MeshGeometry = mStaticMesh.Geometry;
	DrawArgs.Material = mStaticMesh.Material;
	DrawArgs.PrimitiveCBIndex = (UINT)mPrimitiveCBIndex;

	mDrawArgsPoolIds.resize(mStaticMesh.Geometry->DrawArgs.size());
	for (int i = 0; i < mDrawArgsPoolIds.size(); ++i)
	{
		DrawArgs.IndexCount = mStaticMesh.Geometry->DrawArgs[i].IndexCount;
		DrawArgs.StartIndexLocation = mStaticMesh.Geometry->DrawArgs[i].StartIndexLocation;
		DrawArgs.BaseVertexLocation = mStaticMesh.Geometry->DrawArgs[i].BaseVertexLocation;
		std::uint64_t Temp = GetRenderItemManager()->RegisterDrawArgs(DrawArgs);
		mDrawArgsPoolIds[i] = Temp;
	}
}