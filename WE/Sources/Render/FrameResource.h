#pragma once

#include "DirectX/DXUtility.h"
#include "UploadBuffer.h"
#include "Material.h"

struct Light
{
    DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
    float FalloffStart = 1.0f;                          // point/spot light only
    DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
    float FalloffEnd = 10.0f;                           // point/spot light only
    DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
    float SpotPower = 64.0f;                            // spot light only
};

#define MaxLights 16

struct FPassConstants
{
    DirectX::XMFLOAT4X4 View = UDXMath::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = UDXMath::Identity4x4();
};
struct FObjectConstants
{
	DirectX::XMFLOAT4X4 World = UDXMath::Identity4x4();
	DirectX::XMFLOAT4X4 TexTransform = UDXMath::Identity4x4();
};

struct FFrameResource : FNoncopyable
{
public:
	FFrameResource(ID3D12Device* Device, UINT PassCount, UINT ObjectCount, UINT MaterialCount);

	ComPtr<ID3D12CommandAllocator> CommandAllocator;

	unique_ptr<TUploadBuffer<FPassConstants>> PassConstantBuffer;
	unique_ptr<TUploadBuffer<FObjectConstants>> ObjectConstantBuffer;
	unique_ptr<TUploadBuffer<FMaterialConstants>> MaterialConstantBuffer;

	UINT64 Fence = 0;
};