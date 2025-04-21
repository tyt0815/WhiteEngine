#pragma once
#include "SceneRenderer.h"
#include "GameFramework/Object/Actor/Actor.h"
#include "Utility/Class.h"
#include "UploadBuffer.h"

class FForwardShadingSceneRenderer final : public FSceneRenderer
{
private:
    typedef FSceneRenderer Super;
    struct FFSFrameResource : public FFrameResourceBase
    {
    public:
        FFSFrameResource(ID3D12Device* Device);
    };
public:
    virtual void Initialize(ID3D12Device* Device) override;

protected:
    virtual void BuildRootSignature() override;
    virtual void BuildShadersAndInputLayouts() override;
    virtual void BuildPipelineStates(ID3D12Device* Device) override;
    virtual void CreateFrameResources(ID3D12Device* Device) override;
	virtual void Render(ID3D12GraphicsCommandList* CommandList, FFrameResourceBase* FrameResource) override;
private:
	// std::unique_ptr<FCubeSkyRenderer> mSkyCubeMapRenderer;
};