#include "StaticMeshComponent.h"
#include "Render/MeshGeometry.h"
#include "Render/RenderItemManager.h"

extern const int gFrameResourcesNum;

WStaticMeshComponent::WStaticMeshComponent()
{
}

void WStaticMeshComponent::SetStaticMesh(const FStaticMesh& StaticMesh)
{
	for (size_t i = 0; i < ESM_None; ++i)
	{
		EShadingModel ShadingModel = (EShadingModel)i;
		for (size_t j = 0; j < EBM_None; ++j)
		{
			EBlendMode BlendMode = (EBlendMode)j;
			for (size_t k = 0; k < mStaticMeshInfoPoolIds[i][j].size(); ++k)
			{
				GetRenderItemManager()->mStaticMeshInfoPool[ShadingModel][BlendMode].RemoveAt(
					mStaticMeshInfoPoolIds[i][j][k]
				);
			}
		}
	}
	for (size_t i = 0; i < mSubmeshCBIndices.size(); ++i)
	{
		GetRenderItemManager()->mSubmeshInfoPool.RemoveAt(mSubmeshCBIndices[i]);
	}
	mSubmeshCBIndices.clear();

	mStaticMesh = StaticMesh;
	FStaticMeshInfo DrawArgs;
	DrawArgs.MeshGeometry = mStaticMesh.Geometry;
	DrawArgs.Material = mStaticMesh.Material;		// TODO: Submesh당 Material로 변경? 밑에 되있는데...
	DrawArgs.MeshCBIndex = (UINT)mMeshCBIndex;
	DrawArgs.bCastShadow = mbCastShadow;
	for (size_t i = 0; i < mStaticMesh.Geometry->DrawArgs.size(); ++i)
	{
		EShadingModel ShadingModel = mStaticMesh.Material->ShadingModel;
		EBlendMode BlendMode = mStaticMesh.Material->BlendMode;
		DrawArgs.IndexCount = mStaticMesh.Geometry->DrawArgs[i].IndexCount;
		DrawArgs.StartIndexLocation = mStaticMesh.Geometry->DrawArgs[i].StartIndexLocation;
		DrawArgs.BaseVertexLocation = mStaticMesh.Geometry->DrawArgs[i].BaseVertexLocation;
		FSubmeshInfo SubmeshInfo;
		SubmeshInfo.MaterialIndex = mStaticMesh.Material->Type;
		// TODO: 하드코딩
		SubmeshInfo.SkyIrradianceCubeMapIndex = 2;
		SubmeshInfo.SkySpecularCubeMapIndex = 3;
		SubmeshInfo.DirtyFrameCount = gFrameResourcesNum;

		mSubmeshCBIndices.push_back(GetRenderItemManager()->mSubmeshInfoPool.Add(SubmeshInfo));
		DrawArgs.SubmeshCBIndex = mSubmeshCBIndices.back();
		mStaticMeshInfoPoolIds[ShadingModel][BlendMode].push_back(
			GetRenderItemManager()->mStaticMeshInfoPool[ShadingModel][BlendMode].Add(DrawArgs)
		);

	}
}