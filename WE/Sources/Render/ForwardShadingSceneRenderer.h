#pragma once
#include "SceneRenderer.h"
#include "GameFramework/Object/Actor/Actor.h"
#include "Utility/Class.h"
#include "RenderItemManager.h"
#include "CubeSkyRenderer.h"
#include "UploadBuffer.h"





class FForwardShadingSceneRenderer final : public FSceneRenderer
{
private:
    typedef FSceneRenderer Super;
    struct FFrameResource : public FFrameResourceBase
    {

    };
public:
    virtual void Initialize(ID3D12Device* Device) override;
	virtual void Render(const FRenderingData& RenderingData);

protected:
    virtual void BuildShadersAndInputLayouts() override;
    virtual void BuildPipelineStates(ID3D12Device* Device) override;
    virtual void CreateFrameResources(ID3D12Device* Device) override;
    virtual void BuildRootSignature() override;
private:
	void DrawRenderItems(
        FFrameResourceBase* FrameResource,
        ID3D12GraphicsCommandList* CommandList,
        const TPool<FRenderItemInfo>& RenderItems
    );
	std::unique_ptr<FCubeSkyRenderer> mSkyCubeMapRenderer;
};