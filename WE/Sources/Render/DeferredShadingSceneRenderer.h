#pragma once
#include "SceneRenderer.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include "EnvironmentMapRenderer.h"

enum class EDebugScreenMode
{
    EDSM_Default,
    EDSM_DebugAll,
    EDSM_Blur,
    EDSM_GBufferA,
    EDSM_GBufferB,
    EDSM_GBufferC,
    EDSM_Depth
};

class FDeferredShadingSceneRenderer final : public FSceneRenderer
{
    struct FDeferredShadingPassConstantBuffer
    {
        UINT GBufferAIndex;
        UINT GBufferBIndex;
        UINT GBufferCIndex;
        UINT DepthBufferIndex;
        UINT IrradianceMapIndex;
        UINT PrefilteredMapIndex;
        UINT BRDFLUTIndex;
        UINT CBPad1;
    };

private:
    typedef FSceneRenderer Super;
    class FFrameResource : public FFrameResourceBase
    {
    public:
        FFrameResource(ID3D12Device* Device);
    private:
        TUniquePtr<TUploadBuffer<FDeferredShadingPassConstantBuffer>> mDeferredShadingPassCB;
    public:
        inline TUploadBuffer<FDeferredShadingPassConstantBuffer>* GetDeferredShadingPassCB() const
        {
            return mDeferredShadingPassCB.get();
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
    virtual void UpdateFrameBuffers(FFrameResourceBase* FrameResource, const FRenderItemProxy& RenderItemProxy) override;
    void SwitchToDefaultMode();
    void SwitchToDebugAllMode();
    void SwitchToBlurMode();
    void SwitchToGBufferADebugMode();
    void SwitchToGBufferBDebugMode();
    void SwitchToGBufferCDebugMode();
    void SwitchToDepthDebugMode();


    // 해당 버퍼는 처음 실행될때 한번만 업데이트 한다.
    void UpdateDeferredShadingPassCB(TUploadBuffer<FDeferredShadingPassConstantBuffer>* DeferredShadingPassCB);
    virtual void Render(
        ID3D12GraphicsCommandList* CommandList,
        FFrameResourceBase* FrameResourceBase,
        FResource* Resource,
        D3D12_CPU_DESCRIPTOR_HANDLE Rtv,
        D3D12_CPU_DESCRIPTOR_HANDLE Dsv,
        D3D12_VIEWPORT Viewport,
        D3D12_RECT ScissorRect,
        const FRenderItemProxy& RenderItemProxy
    ) override;

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

    void DrawGBuffers(ID3D12GraphicsCommandList* CommandList, FFrameResource* FrameResource, const FRenderItemProxy& RenderItemProxy);

    TUniquePtr<FRenderTarget> mGBufferA;
    TUniquePtr<FRenderTarget> mGBufferB;
    TUniquePtr<FRenderTarget> mGBufferC;
    TUniquePtr<FDepthStencil> mGBufferDepthStencil;
    TUniquePtr<FEnvironmentMapRenderer> mEnvironmentMapRenderer;

    EDebugScreenMode mScreenMode = EDebugScreenMode::EDSM_Default;
};