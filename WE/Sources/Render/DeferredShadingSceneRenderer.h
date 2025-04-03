#pragma once
#include "SceneRenderer.h"

class FDeferredShadingSceneRenderer final : public FSceneRenderer
{
    typedef FSceneRenderer Super;
public:
    virtual void Render(const FRenderingData& RenderingData) override;

protected:
    virtual void BuildShadersAndInputLayouts() override;
    virtual void BuildPipelineStates(ID3D12Device* Device) override;
    virtual void CreateFrameResources(ID3D12Device* Device) override;
    virtual void BuildRootSignature() override;
    virtual void UpdateFrameBuffers(FFrameResourceBase* FrameResource) override;
};