#pragma once

#include "UploadBuffer.h"
#include "Material.h"
#include "DirectX/DXUtility.h"
#include "Utility/Class.h"

struct FLight
{
    DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
    float FalloffStart = 1.0f;                          // point/spot light only
    DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
    float FalloffEnd = 10.0f;                           // point/spot light only
    DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
    float SpotPower = 64.0f;                            // spot light only
};

#define MaxLights 16

// register(b0)
struct FPassConstants
{
    DirectX::XMFLOAT4X4 View = FDXMath::Identity4x4();
    DirectX::XMFLOAT4X4 InvView = FDXMath::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = FDXMath::Identity4x4();
    DirectX::XMFLOAT4X4 InvProj = FDXMath::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = FDXMath::Identity4x4();
    DirectX::XMFLOAT4X4 InvViewProj = FDXMath::Identity4x4();
    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1 = 0.0f;
    DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;

    DirectX::XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Fog Info
    DirectX::XMFLOAT4 FogColor;
    float FogStart;
    float FogRange;
    DirectX::XMFLOAT2 cbPerObjectPad2;

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    FLight Lights[MaxLights];
};

// register(b1)
struct FObjectConstants
{
	DirectX::XMFLOAT4X4 World = FDXMath::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = FDXMath::Identity4x4();
};

struct FFrameResource : FNoncopyable
{
public:
	FFrameResource(ID3D12Device* Device, UINT PassCount, UINT ObjectCount, UINT MaterialCount);

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;

	std::unique_ptr<TUploadBuffer<FPassConstants>> PassConstantBuffer;
	std::unique_ptr<TUploadBuffer<FObjectConstants>> ObjectConstantBuffer;
	std::unique_ptr<TUploadBuffer<FMaterialConstants>> MaterialConstantBuffer;

	UINT64 Fence = 0;
};