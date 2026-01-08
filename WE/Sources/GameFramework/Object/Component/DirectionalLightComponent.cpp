#include "DirectionalLightComponent.h"
#include "GameFramework/Object/World/World.h"

extern const int gFrameResourcesNum;

WDirectionalLightComponent::WDirectionalLightComponent()
{
	mShadowMap = std::make_unique<FDepthStencil>(1920 * 2, 1080 * 2);
	
}

void WDirectionalLightComponent::Update()
{
	WWorld* World = GetWorld();
	size_t ProxyIndex = World->AllocateDirectionalLightCbProxy();
	FDirectionalLightProxy* Proxy = World->GetDirectionalLightProxy(ProxyIndex);

	DirectX::XMFLOAT4 WorldQuatRotation = GetWorldQuatRotation();
	DirectX::XMVECTOR WorldQuat = DirectX::XMLoadFloat4(&WorldQuatRotation);
	DirectX::XMVECTOR XAxis = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR DirectionVector = DirectX::XMVector3Rotate(XAxis, WorldQuat);
	DirectX::XMStoreFloat3(&Proxy->Direction, DirectionVector);

	Proxy->Color = mColor;

	Proxy->bCastShadow = mbCastShadow;
	Proxy->ShadowMap = mShadowMap.get();

	Super::Update();
}