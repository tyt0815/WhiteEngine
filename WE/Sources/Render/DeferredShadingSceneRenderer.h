#pragma once
#include "SceneRenderer.h"

class FDeferredShadingSceneRenderer final : public FSceneRenderer
{
    typedef FSceneRenderer Super;
public:

protected:
    virtual void BuildShadersAndInputLayouts() override;
    virtual void BuildPipelineStates(ID3D12Device* Device) override;
    virtual void CreateFrameResources(ID3D12Device* Device) override;
    virtual void BuildRootSignature() override;
    virtual void UpdateFrameBuffers(FFrameResourceBase* FrameResource) override;
    virtual void Render(ID3D12GraphicsCommandList* CommandList, FFrameResourceBase* FrameResource) override;
};