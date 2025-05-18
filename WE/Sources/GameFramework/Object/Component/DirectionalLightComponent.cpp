#include "DirectionalLightComponent.h"
#include "Render/RenderItemManager.h"

extern const int gFrameResourcesNum;

WDirectionalLightComponent::WDirectionalLightComponent():
	mDirectionalLightInfoPoolIndex(GetRenderItemManager()->mDirectionalLightInfoPool.Register({}))
{
	mShadowMap = std::make_unique<FDepthStencil>(1920 * 2, 1080 * 2);
}

void WDirectionalLightComponent::Update()
{
	TPool<FDirectionalLightInfo>& LightInfoPool = GetRenderItemManager()->mDirectionalLightInfoPool;
	FDirectionalLightInfo LightInfo = LightInfoPool.GetItem(mDirectionalLightInfoPoolIndex);
	if (mbDirty)
	{
		LightInfo.DirtyFrameCount = gFrameResourcesNum;
		DirectX::XMFLOAT4 WorldQuatRotation = GetWorldQuatRotation();
		DirectX::XMVECTOR WorldQuat = DirectX::XMLoadFloat4(&WorldQuatRotation);
		DirectX::XMVECTOR XAxis = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR DirectionVector = DirectX::XMVector3Rotate(XAxis, WorldQuat);
		DirectX::XMStoreFloat3(&LightInfo.Direction, DirectionVector);
	}
	if (bColorChanged)
	{
		bColorChanged = false;
		LightInfo.DirtyFrameCount = gFrameResourcesNum;
		LightInfo.Color = mColor;
	}

	if (LightInfo.DirtyFrameCount == gFrameResourcesNum || mbCastShadow != LightInfo.bCastShadow)
	{
		LightInfo.bCastShadow = mbCastShadow;
		LightInfo.ShadowMap = mShadowMap.get();
		LightInfoPool.SetItem(mDirectionalLightInfoPoolIndex, LightInfo);
	}
	Super::Update();
}