#pragma once
#include <array>
#include <d3d12.h>
#include <memory>
#include <unordered_map>
#include <wrl.h>
#include "Material.h"
#include "RenderItemManager.h"
#include "UploadBuffer.h"
#include "Utility/Class.h"
#include "Utility/String.h"

extern const int gFrameResourcesNum;
constexpr int FRAME_RESOURCES_NUM = 3;

constexpr int DIR_LIGHTS_NUM = 32;
// CosntantBuffer
constexpr int MESH_CB_NUM = 512;
constexpr int SUBMESH_CB_NUM = 1024;

struct FDirectionalLight
{
    XMFLOAT3 Direction;
    UINT Pad1;
    XMFLOAT3 Color;
    UINT Pad2;
};

// register(b0)
struct FPassConstantBuffer
{
    DirectX::XMFLOAT4X4 View = FDXMath::Identity4x4();
    DirectX::XMFLOAT4X4 InvView = FDXMath::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = FDXMath::Identity4x4();
    DirectX::XMFLOAT4X4 InvProj = FDXMath::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = FDXMath::Identity4x4();
    DirectX::XMFLOAT4X4 InvViewProj = FDXMath::Identity4x4();
    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    UINT IndirectSpecularIntegralTextureIndex;
    DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;

    // Fog Info
    DirectX::XMFLOAT4 FogColor;
    float FogStart;
    float FogRange;
    UINT DirLightNum;
    UINT Pad1;
};

// register(b1)
struct FMeshConstantBuffer
{
    DirectX::XMFLOAT4X4 World = FDXMath::Identity4x4();
    DirectX::XMFLOAT4X4 InvTransposeWorld;
};

// register(b2)
struct FSubmeshConstantBuffer
{
    UINT MaterialIndex;
    UINT SkyIrradianceCubeMapIndex;
    UINT SkySpecularCubeMapIndex;
    UINT Pad1;
};

// register(t0, space1)
struct FMaterialStructuredBuffer
{
    UINT AbeldoTextureIndex;
    UINT MetallicTextureIndex;
    UINT RoughnessTextureIndex;
    UINT NormalTextureIndex;
    DirectX::XMFLOAT4X4 MatTransform = FDXMath::Identity4x4();
};

class FFrameResourceBase : FNoncopyable
{
public:
    FFrameResourceBase(ID3D12Device* Device);
    FFrameResourceBase() = delete;
    virtual ~FFrameResourceBase();
    void Flush();

private:
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;
    std::unique_ptr<TUploadBuffer<FPassConstantBuffer>> mPassConstantBuffer;
    std::unique_ptr<TUploadBuffer<FMeshConstantBuffer>> mMeshConstantBuffer;
    std::unique_ptr<TUploadBuffer<FSubmeshConstantBuffer>> mSubmeshConstantBuffer;
    std::unique_ptr<TUploadBuffer<FMaterialStructuredBuffer>> mMaterialStructuredBuffer;
    std::unique_ptr<TUploadBuffer<FDirectionalLight>> mDirectionalLightStructuredBuffer;
    UINT64 mFenceCount = 0;

public:
    inline ID3D12CommandAllocator* GetCommandAllocatorPtr() const
    {
        return mCommandAllocator.Get();
    }
    inline UINT64 GetFenceCount() const
    {
        return mFenceCount;
    }
    inline void SetFenceCount(UINT64 FenceCount)
    {
        mFenceCount = FenceCount;
    }
    inline TUploadBuffer<FPassConstantBuffer>* GetPassCB() const
    {
        return mPassConstantBuffer.get();
    }
    inline TUploadBuffer<FMeshConstantBuffer>* GetMeshCB() const
    {
        return mMeshConstantBuffer.get();
    }
    inline TUploadBuffer<FSubmeshConstantBuffer>* GetSubmeshCB() const
    {
        return mSubmeshConstantBuffer.get();
    }
    inline TUploadBuffer<FMaterialStructuredBuffer>* GetMaterialSB() const
    {
        return mMaterialStructuredBuffer.get();
    }
    inline TUploadBuffer<FDirectionalLight>* GetDirectionalLightSB() const
    {
        return mDirectionalLightStructuredBuffer.get();
    }
};

class FSceneRenderer : FNoncopyable
{
public:
	FSceneRenderer();
	virtual ~FSceneRenderer() = default;
    virtual void Initialize(ID3D12Device* Device);
    void Tick();
    virtual void Destroy();

protected:
    virtual void CreateFrameResources(ID3D12Device* Device) = 0;
    template<typename T>
    void CreateFrameResources_Internal(ID3D12Device* Device);
    virtual void BuildRootSignature();
    virtual void BuildShadersAndInputLayouts();
    virtual void BuildPipelineStates(ID3D12Device* Device);
    virtual void UpdateFrameBuffers(FFrameResourceBase* FrameResource);
    virtual void Render(ID3D12GraphicsCommandList* CommandList, FFrameResourceBase* FrameResource) = 0;
    void DrawRenderItems(
        FFrameResourceBase* FrameResource,
        ID3D12GraphicsCommandList* CommandList,
        const TPool<FRenderItemInfo>& RenderItems
    );
    // BackBuffer와 DepthStencilBuffer를 쓰기 가능한 상태로 전이하고, Clear한다.
    void ReadyBackBuffer(ID3D12GraphicsCommandList* CommandList);

    // BackBuffer와 DepthStencilBuffer를 Present하기 위한 상태로 전이한다.
    void FinishBackBuffer(ID3D12GraphicsCommandList* CommandList);

    std::array<std::unique_ptr<FFrameResourceBase>, FRAME_RESOURCES_NUM> mFrameResources;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> mRootSignatures;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> mShaders;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> mPipelineStates;
    std::unordered_map<std::string, std::vector<D3D12_INPUT_ELEMENT_DESC>> mInputLayouts;
    int mTargetFrameResourceIndex = 0;
    bool bWireFrame = false;

private:
    void UpdateTargetFrameResource();
    void SwitchToNextFrameResource();
    void UpdatePassCB(TUploadBuffer<FPassConstantBuffer>* PassConstantBuffer);
    void UpdateMeshCB(TUploadBuffer<FMeshConstantBuffer>* MeshConstantBuffer);
    void UpdateSubmeshCB(TUploadBuffer<FSubmeshConstantBuffer>* SubmeshConstantBuffer);
    void UpdateMaterialSB(TUploadBuffer<FMaterialStructuredBuffer>* MaterialStructuredBuffer);
    void UpdateDirectionalLightSB(TUploadBuffer<FDirectionalLight>* DirectionalLightStructuredBuffer);

public:
    inline FFrameResourceBase* GetTargetFrameResource() const
    {
        return mFrameResources[mTargetFrameResourceIndex].get();
    }
};

template<typename T>
inline void FSceneRenderer::CreateFrameResources_Internal(ID3D12Device* Device)
{
    for (int i = 0; i < mFrameResources.size(); ++i)
    {
        mFrameResources[i] = std::make_unique<T>(Device);
    }
}
