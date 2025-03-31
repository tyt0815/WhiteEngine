#include "PrimitiveComponent.h"
#include "Render/FrameResource.h"

WPrimitiveComponent::WPrimitiveComponent() :
	mPrimitiveCBIndex(GetFrameResourceManager()->RegisterMeshCBInfo({}))
{

}

void WPrimitiveComponent::Update()
{
	Super::Update();

	// 직접적으로 업로드될 데이터를 FrameResourceManager에 저장
	if (mbDirty)
	{
		FMeshCBInfo MeshCBInfo;
		MeshCBInfo.DirtyFrameCount = FrameResourcesNum;
		XMFLOAT4X4 World = GetWorldMatrix();
		XMMATRIX WorldMat = XMLoadFloat4x4(&World);
		
		XMMATRIX InvTransposeWorld = FDXMath::InverseTranspose(WorldMat);
		XMStoreFloat4x4(&MeshCBInfo.MeshCB.World, XMMatrixTranspose(WorldMat));
		XMStoreFloat4x4(&MeshCBInfo.MeshCB.InvTransposeWorld, XMMatrixTranspose(InvTransposeWorld));
		GetFrameResourceManager()->SetMeshCBInfo(mPrimitiveCBIndex, MeshCBInfo);
	}
}
