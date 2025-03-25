#include "StaticMeshComponent.h"
#include "Render/MeshGeometry.h"
#include "Render/RenderItemManager.h"
#include "Render/FrameResource.h"

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
			for (size_t k = 0; k < mRenderItemInfoPoolIds[i][j].size(); ++k)
			{
				GetRenderItemManager()->RemoveRenderItem(ShadingModel, BlendMode, mRenderItemInfoPoolIds[i][j][k]);
			}
		}
	}
	mSubmeshCBInfoPoolIds.clear();

	mStaticMesh = StaticMesh;
	FRenderItemInfo DrawArgs;
	DrawArgs.MeshGeometry = mStaticMesh.Geometry;
	DrawArgs.Material = mStaticMesh.Material;
	DrawArgs.MeshCBIndex = (UINT)mPrimitiveCBIndex;

	for (size_t i = 0; i < mStaticMesh.Geometry->DrawArgs.size(); ++i)
	{
		EShadingModel ShadingModel = mStaticMesh.Material->ShadingModel;
		EBlendMode BlendMode = mStaticMesh.Material->BlendMode;
		DrawArgs.IndexCount = mStaticMesh.Geometry->DrawArgs[i].IndexCount;
		DrawArgs.StartIndexLocation = mStaticMesh.Geometry->DrawArgs[i].StartIndexLocation;
		DrawArgs.BaseVertexLocation = mStaticMesh.Geometry->DrawArgs[i].BaseVertexLocation;
		FSubmeshConstantBuffer SubmeshCB;
		SubmeshCB.MaterialIndex = mStaticMesh.Material->Type;
		// TODO: 하드코딩되어있음
		SubmeshCB.SkyIrradianceCubeMapIndex = 2;
		FSubmeshCBInfo SubmeshCBInfo;
		SubmeshCBInfo.SubmeshCB = SubmeshCB;
		SubmeshCBInfo.DirtyFrameCount = FrameResourcesNum;
		mSubmeshCBInfoPoolIds.push_back(GetFrameResourceManager()->RegisterSubmeshCBInfo(SubmeshCBInfo));
		DrawArgs.SubmeshCBIndex = mSubmeshCBInfoPoolIds.back();
		size_t Temp = GetRenderItemManager()->RegisterRenderItem(ShadingModel, BlendMode, DrawArgs);
		mRenderItemInfoPoolIds[ShadingModel][BlendMode].push_back(Temp);

	}
}