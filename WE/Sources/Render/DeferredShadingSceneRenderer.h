#pragma once
#include "SceneRenderer.h"

class FDeferredShadingSceneRenderer final : public FSceneRenderer
{
    typedef FSceneRenderer Super;
public:
    virtual void Render();

protected:
    virtual void BuildShadersAndInputLayouts() override;
    virtual void BuildPipelineStates() override;
    virtual void CreateFrameResources() override;
    virtual void BuildRootSignature() override;
    virtual void UpdateFrameBuffers(FFrameResource* FrameResource) override;
};