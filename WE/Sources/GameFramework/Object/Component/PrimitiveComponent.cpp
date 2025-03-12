#include "PrimitiveComponent.h"
#include "Render/FrameResource.h"

WPrimitiveComponent::WPrimitiveComponent() :
	mPrimitiveCBIndex(GetFrameResourceManager()->RegisterObjectCBInfo({}))
{

}

void WPrimitiveComponent::Update()
{
	Super::Update();

	// 직접적으로 업로드될 데이터를 FrameResourceManager에 저장
	if (mbDirty)
	{
		FObjectCBInfo ObjectCBInfo;
		ObjectCBInfo.DirtyFrameCount = FrameResourcesNum;
		XMFLOAT4X4 World = GetWorldMatrix();
		XMMATRIX WorldMat = XMLoadFloat4x4(&World);
		XMStoreFloat4x4(&ObjectCBInfo.ObjectConstants.World, XMMatrixTranspose(WorldMat));
		GetFrameResourceManager()->SetObjectCBInfo(mPrimitiveCBIndex, ObjectCBInfo);
	}
}
