#pragma once
#include "SceneRenderer.h"
#include "GameFramework/Object/Actor/Actor.h"
#include "Utility/Class.h"
#include "RenderItemManager.h"
#include "CubeSkyRenderer.h"
#include "UploadBuffer.h"



constexpr int DIR_LIGHTS_NUM = 1;

// CosntantBuffer
constexpr int MESH_CB_NUM = 512;
constexpr int SUBMESH_CB_NUM = 1024;

class FForwardShadingSceneRenderer : public FSceneRenderer
{
    typedef FSceneRenderer Super;
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

    class FForwardShadingSceneFrameResource : public FFrameResource
    {
    public:
        FForwardShadingSceneFrameResource();
    private:
        std::unique_ptr<TUploadBuffer<FPassConstantBuffer>> mPassConstantBuffer;
        std::unique_ptr<TUploadBuffer<FMeshConstantBuffer>> mMeshConstantBuffer;
        std::unique_ptr<TUploadBuffer<FSubmeshConstantBuffer>> mSubmeshConstantBuffer;
        std::unique_ptr<TUploadBuffer<FMaterialStructuredBuffer>> mMaterialConstantBuffer;

    public:
        inline TUploadBuffer<FPassConstantBuffer>* GetPassCB()
        {
            return mPassConstantBuffer.get();
        }
        inline TUploadBuffer<FMeshConstantBuffer>* GetMeshCB()
        {
            return mMeshConstantBuffer.get();
        }
        inline TUploadBuffer<FSubmeshConstantBuffer>* GetSubmeshCB()
        {
            return mSubmeshConstantBuffer.get();
        }
        inline TUploadBuffer<FMaterialStructuredBuffer>* GetMaterialSB()
        {
            return mMaterialConstantBuffer.get();
        }
    };
public:
	FForwardShadingSceneRenderer();
	virtual void Render();

private:
    virtual void CreateFrameResources() override;
    virtual void BuildRootSignature() override;
    virtual void BuildShadersAndInputLayouts() override;
    virtual void BuildPipelineStates() override;
    virtual void UpdateFrameBuffers(FFrameResource* FrameResource) override;
    void UpdatePassCB(TUploadBuffer<FPassConstantBuffer>* PassConstantBuffer);
    void UpdateMeshCB(TUploadBuffer<FMeshConstantBuffer>* MeshConstantBuffer);
    void UpdateSubmeshCB(TUploadBuffer<FSubmeshConstantBuffer>* SubmeshConstantBuffer);
    void UpdateMaterialCB(TUploadBuffer<FMaterialStructuredBuffer>* MaterialStructuredBuffer);
	void DrawRenderItems(
        FForwardShadingSceneFrameResource* FrameResource,
        ID3D12GraphicsCommandList* CommandList,
        const TPool<FRenderItemInfo>& RenderItems
    );
	std::unique_ptr<FCubeSkyRenderer> mSkyCubeMapRenderer;
};