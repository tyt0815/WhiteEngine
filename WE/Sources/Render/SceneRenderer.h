#pragma once
#include <array>
#include <d3d12.h>
#include <memory>
#include <unordered_map>
#include <wrl.h>
#include "Material.h"
#include "DepthStencil.h"
#include "GausiaanBlurFilter.h"
#include "RenderItemManager.h"
#include "UploadBuffer.h"
#include "Utility/Class.h"
#include "Utility/String.h"

extern const int gFrameResourcesNum;
constexpr int FRAME_RESOURCES_NUM = 3;

constexpr int DIR_LIGHTS_NUM = 3;
// CosntantBuffer
constexpr int MESH_CB_NUM = 512;
constexpr int SUBMESH_CB_NUM = 1024;

class FRenderItemManager;

struct FDirectionalLightSB
{
    XMFLOAT4X4 LightViewProj;
    XMFLOAT4X4 ShadowTransform;
    XMFLOAT3 Direction;
    UINT ShadowMapIndex;
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
    UINT Pad1;
    UINT Pad2;
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

// register(b3)
struct FLightInfoConstantBuffer
{
    UINT DirectionalLightNum;
    UINT PointLightNum;
    UINT SpotLightNum;
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
    std::unique_ptr<TUploadBuffer<FLightInfoConstantBuffer>> mLightInfoConstantBuffer;
    std::unique_ptr<TUploadBuffer<FMaterialStructuredBuffer>> mMaterialStructuredBuffer;
    std::unique_ptr<TUploadBuffer<FDirectionalLightSB>> mDirectionalLightStructuredBuffer;
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
    inline TUploadBuffer<FLightInfoConstantBuffer>* GetLightInfoCB() const
    {
        return mLightInfoConstantBuffer.get();
    }
    inline TUploadBuffer<FMaterialStructuredBuffer>* GetMaterialSB() const
    {
        return mMaterialStructuredBuffer.get();
    }
    inline TUploadBuffer<FDirectionalLightSB>* GetDirectionalLightSB() const
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
    void BuildShadowMapPassRootSignature();
    virtual void BuildShadersAndInputLayouts();
    void BuildShadowMapShaders();
    virtual void BuildPipelineStates(ID3D12Device* Device);
    void BuildShadowMapPassPipelineStates(ID3D12Device* Device);
    virtual void UpdateFrameBuffers(FFrameResourceBase* FrameResource);
    virtual void Render(
        ID3D12GraphicsCommandList* CommandList,
        FFrameResourceBase* FrameResourceBase,
        FResource* Resource,
        D3D12_CPU_DESCRIPTOR_HANDLE Rtv,
        D3D12_CPU_DESCRIPTOR_HANDLE Dsv,
        D3D12_VIEWPORT Viewport,
        D3D12_RECT ScissorRect
    ) = 0;

    void DrawRectPass(
        ID3D12GraphicsCommandList* CommandList,
        UINT TextureSRVIndex,
        D3D12_CPU_DESCRIPTOR_HANDLE Rtv,
        D3D12_CPU_DESCRIPTOR_HANDLE Dsv,
        const D3D12_VIEWPORT& Viewport,
        std::string PipelineStateName = "DrawRectPass"
    );
    
    void DrawShadowMap(ID3D12GraphicsCommandList* CommandList, FFrameResourceBase* FrameResource);

    void ClearRenderTargetAndDepthStencil(
        ID3D12GraphicsCommandList* CommandList,
        D3D12_CPU_DESCRIPTOR_HANDLE& Rtv,
        D3D12_CPU_DESCRIPTOR_HANDLE& Dsv,
        const D3D12_RECT& ScissorRect
    );

    void DrawStaticMeshs(
        ID3D12GraphicsCommandList* CommandList,
        ID3D12Resource* MeshConstantBuffer,
        ID3D12Resource* SubmeshConstantBuffer,
        FRenderItemManager* RIM
    );
    
    void DrawStaticMesh(
        ID3D12GraphicsCommandList* CommandList,
        ID3D12Resource* MeshConstantBuffer,
        ID3D12Resource* SubmeshConstantBuffer,
        const FStaticMeshInfo& StaticMeshInfo
    );

    // BackBuffer와 DepthStencilBuffer를 쓰기 가능한 상태로 전이하고, Clear한다.
    void ReadyBackBuffer(ID3D12GraphicsCommandList* CommandList);

    // BackBuffer와 DepthStencilBuffer를 Present하기 위한 상태로 전이한다.
    void FinishBackBuffer(ID3D12GraphicsCommandList* CommandList);

    std::unique_ptr<FDepthStencil> mShadowMap;
    std::unique_ptr<FGaussianBlurFilter> mGaussianBlurFilter;
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
    void UpdateLightInfoCB(TUploadBuffer<FLightInfoConstantBuffer>* LightInfoConstantBuffer);
    void UpdateMaterialSB(TUploadBuffer<FMaterialStructuredBuffer>* MaterialStructuredBuffer);
    void UpdateDirectionalLightSB(TUploadBuffer<FDirectionalLightSB>* DirectionalLightStructuredBuffer);

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
