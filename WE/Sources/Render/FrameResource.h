#pragma once
#include <memory>
#include <vector>
#include "Material.h"
#include "UploadBuffer.h"
#include "DirectX/DXUtility.h"
#include "DirectX/DXMath.h"
#include "Utility/Class.h"
#include "Utility/Pool.h"

constexpr std::uint32_t FRAME_RESOURCES_NUM = 3;
constexpr std::uint32_t PASS_COUNT = 1;

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
    // TODO: TexTransform¿∫ ªË¡¶
    DirectX::XMFLOAT4X4 TexTransform = FDXMath::Identity4x4();
};

struct FObjectCBInfo
{
    FObjectConstants ObjectConstants;
    std::uint64_t DirtyFrameCount;
};

// register(b2)
struct FMaterialConstants
{
    DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
    float Roughness = 0.25f;

    // Used in texture mapping.
    DirectX::XMFLOAT4X4 MatTransform = FDXMath::Identity4x4();
};

struct FFrameResource : FNoncopyable
{
public:
    FFrameResource();

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;

	std::unique_ptr<TUploadBuffer<FPassConstants>> PassConstantBuffer;
	std::unique_ptr<TUploadBuffer<FObjectConstants>> ObjectConstantBuffer;
	std::unique_ptr<TUploadBuffer<FMaterialConstants>> MaterialConstantBuffer;

	UINT64 Fence = 0;
private:
    void Update();

    friend class FFrameResourceManager;
};

class FFrameResourceManager
{
    SINGLETON(FFrameResourceManager)
public:
    void Tick();
	void FlushCommandQueues();

private:
    void SetTargetFrameResource();
    void BuildRootSignature();
    void UpdatePassCB();
    void UpdateObjectCB();
    void UpdateMaterialCB();
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
    FFrameResource* mTargetFrameResource = nullptr;
    std::vector<std::unique_ptr<FFrameResource>> mFrameResources;
    std::uint32_t mTargetFrameResourceIndex = 0;
    TPool<FObjectCBInfo> mObjectCBInfoPool;

public:
    inline FFrameResource* GetTargetFrameResource() const
    {
        return mTargetFrameResource;
    }
    inline void SetPassCBOfTargetFrame(int i, const FPassConstants& PassCB)
    {
        mTargetFrameResource->PassConstantBuffer->CopyData(i, PassCB);
    }
    inline void SetObjectCBOfTargetFrame(int i, const FObjectConstants& ObjectCB)
    {
        mTargetFrameResource->ObjectConstantBuffer->CopyData(i, ObjectCB);
    }
    inline void SetMaterialCBOfTargetFrame(int i, const FMaterialConstants& MaterialCB)
    {
        mTargetFrameResource->MaterialConstantBuffer->CopyData(i, MaterialCB);
    }
    inline ID3D12RootSignature* GetRootSignaturePtr() const
    {
        return mRootSignature.Get();
    }
    inline std::uint64_t RegisterObjectCBInfo(const FObjectCBInfo& ObjectCBInfo)
    {
        return mObjectCBInfoPool.Register(ObjectCBInfo);
    }
    inline void RemoveObjectCBInfo(std::uint64_t Id)
    {
        mObjectCBInfoPool.Remove(Id);
    }
    inline void SetObjectCBInfo(std::uint64_t Id, const FObjectCBInfo& ObjectCBInfo)
    {
        mObjectCBInfoPool.SetItem(Id, ObjectCBInfo);
    }
};

inline FFrameResourceManager* GetFrameResourceManager()
{
    return FFrameResourceManager::GetInstance();
}