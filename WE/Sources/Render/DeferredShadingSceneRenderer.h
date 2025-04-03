#pragma once
#include "SceneRenderer.h"
#include "RenderTarget.h"

class FDeferredShadingSceneRenderer final : public FSceneRenderer
{
private:
    typedef FSceneRenderer Super;
    class FFrameResource : public FFrameResourceBase
    {
    public:
        FFrameResource(ID3D12Device* Device);
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
    virtual void BuildPipelineStates(ID3D12Device* Device) override;
    virtual void UpdateFrameBuffers(FFrameResourceBase* FrameResource) override;
    virtual void Render(ID3D12GraphicsCommandList* CommandList, FFrameResourceBase* FrameResource) override;

    std::unique_ptr<FRenderTarget> mGBuffers;
};