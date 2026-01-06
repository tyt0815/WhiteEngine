#include "PrimitiveComponent.h"
#include "Render/RenderItemManager.h"

extern const int gFrameResourcesNum;

WPrimitiveComponent::WPrimitiveComponent() :
	mMeshCBIndex(GetRenderItemManager()->mMeshInfoPool.Add(FMeshInfo()))
{

}

void WPrimitiveComponent::Update()
{
	Super::Update();

	// 직접적으로 업로드될 데이터를 FrameResourceManager에 저장
	if (mbDirty)
	{
		FMeshInfo MeshInfo;
		MeshInfo.World = GetWorldMatrix();
		MeshInfo.DirtyFrameCount = gFrameResourcesNum;
		GetRenderItemManager()->mMeshInfoPool[mMeshCBIndex] = MeshInfo;
	}
}
