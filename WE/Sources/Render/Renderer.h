#pragma once

#include <d3d12.h>
#include <array>
#include <unordered_map>
#include <memory>
#include <vector>
#include "GameFramework/Object/Actor/Actor.h"
#include "Utility/Class.h"
#include "RenderItemManager.h"
#include "CubeSkyRenderer.h"
#include "UploadBuffer.h"

extern const int gFrameResourcesNum;
constexpr std::uint32_t FRAME_RESOURCES_NUM = 3;
constexpr int DIR_LIGHTS_NUM = 1;

// CosntantBuffer
constexpr int MESH_CB_NUM = 512;
constexpr int SUBMESH_CB_NUM = 1024;

class FRenderer : FNoncopyable
{
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
        FDirectionalLight DirectionalLights[DIR_LIGHTS_NUM];
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

    struct FFrameResource : FNoncopyable
    {
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;

        std::unique_ptr<TUploadBuffer<FPassConstantBuffer>> PassConstantBuffer;
        std::unique_ptr<TUploadBuffer<FMeshConstantBuffer>> MeshConstantBuffer;
        std::unique_ptr<TUploadBuffer<FSubmeshConstantBuffer>> SubmeshConstantBuffer;
        std::unique_ptr<TUploadBuffer<FMaterialStructuredBuffer>> MaterialConstantBuffer;

        UINT64 Fence = 0;
    };
public:
	FRenderer();
    virtual ~FRenderer();
	virtual void Render();

private:
    void CreateFrameResources();
    void InitializeFrameResource(FFrameResource* FrameResource);
    void BuildRootSignature();
    void UpdateTargetFrameResource();
    void SetTargetFrameResource();
    void FlushFrameResourceQueue(FFrameResource* FrameResource);
    void UpdatePassCB(TUploadBuffer<FPassConstantBuffer>* PassConstantBuffer);
    void UpdateMeshCB(TUploadBuffer<FMeshConstantBuffer>* MeshConstantBuffer);
    void UpdateSubmeshCB(TUploadBuffer<FSubmeshConstantBuffer>* SubmeshConstantBuffer);
    void UpdateMaterialCB(TUploadBuffer<FMaterialStructuredBuffer>* MaterialStructuredBuffer);
	void DrawRenderItems(FFrameResource* FrameResource, ID3D12GraphicsCommandList* CommandList, const TPool<FRenderItemInfo>& RenderItems);
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
	std::unique_ptr<FCubeSkyRenderer> mSkyCubeMapRenderer;
    std::array<std::unique_ptr<FFrameResource>, FRAME_RESOURCES_NUM> mFrameResources;
    FFrameResource* mTargetFrameResource;
    int mTargetFrameResourceIndex = 0;
	bool bWireFrame = false;
};