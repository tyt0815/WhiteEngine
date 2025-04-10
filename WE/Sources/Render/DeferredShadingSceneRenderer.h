#pragma once
#include "SceneRenderer.h"
#include "RenderTarget.h"

class FDeferredShadingSceneRenderer final : public FSceneRenderer
{
    struct FGBufferInfoConstantBuffer
    {
        UINT GBufferAIndex;
        UINT GBufferBIndex;
        UINT GBufferCIndex;
        UINT DepthBufferIndex;
    };
private:
    typedef FSceneRenderer Super;
    class FFrameResource : public FFrameResourceBase
    {
    public:
        FFrameResource(ID3D12Device* Device);
    private:
        std::unique_ptr<TUploadBuffer<FGBufferInfoConstantBuffer>> mGBufferInfoCB;
    public:
        inline TUploadBuffer<FGBufferInfoConstantBuffer>* GetGBufferInfoCB() const
        {
            return mGBufferInfoCB.get();
        }
    };
public:
    FDeferredShadingSceneRenderer();
    virtual void Initialize(ID3D12Device* Device) override;

private:
    virtual void CreateFrameResources(ID3D12Device* Device) override;
    virtual void BuildRootSignature() override;
    void BuildDeferredShadingPassRootSignature();
    void BuildGBufferRootSignature();
    virtual void BuildShadersAndInputLayouts() override;
    void BuildDeferredShadingPassShaders();
    void BuildGBufferPassShaders();
    void BuildDebugPassShaders();
    virtual void BuildPipelineStates(ID3D12Device* Device) override;
    void BuildDeferredShadingPassPipelineState(ID3D12Device* Device);
    void BuildGBufferPassPipelineState(ID3D12Device* Device);
    void BuildDebugPassPipelineStates(ID3D12Device* Device);
    virtual void UpdateFrameBuffers(FFrameResourceBase* FrameResource) override;

    // 해당 버퍼는 처음 실행될때 한번만 업데이트 한다.
    void UpdateGBufferInfoCB(TUploadBuffer<FGBufferInfoConstantBuffer>* GBufferInfoCB);
    virtual void Render(ID3D12GraphicsCommandList* CommandList, FFrameResourceBase* FrameResourceBase) override;

    void DrawDebugGBuffers(
        ID3D12GraphicsCommandList* CommandList,
        D3D12_CPU_DESCRIPTOR_HANDLE Rtv,
        D3D12_CPU_DESCRIPTOR_HANDLE Dsv,
        const D3D12_VIEWPORT& Viewport
    );

    void DrawDeferredShadingPass(
        ID3D12GraphicsCommandList* CommandList,
        D3D12_CPU_DESCRIPTOR_HANDLE Rtv,
        D3D12_CPU_DESCRIPTOR_HANDLE Dsv,
        const D3D12_VIEWPORT& Viewport,
        FFrameResource* FrameResource
    );

    void DrawGBuffers(ID3D12GraphicsCommandList* CommandList, FFrameResource* FrameResource);

    std::unique_ptr<FRenderTarget> mGBuffers;
};