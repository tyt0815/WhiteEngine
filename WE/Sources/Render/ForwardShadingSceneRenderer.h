#pragma once
#include "SceneRenderer.h"
#include "GameFramework/Object/Actor/Actor.h"
#include "Utility/Class.h"
#include "RenderItemManager.h"
#include "CubeSkyRenderer.h"
#include "UploadBuffer.h"





class FForwardShadingSceneRenderer final : public FSceneRenderer
{
    typedef FSceneRenderer Super;
public:
    virtual void Initialize(
        ID3D12Device* Device,
        ID3D12CommandQueue* CommandQueue,
        ID3D12GraphicsCommandList* CommandList
    ) override;
	virtual void Render();

protected:
    virtual void BuildShadersAndInputLayouts() override;
    virtual void BuildPipelineStates() override;
private:
	void DrawRenderItems(
        FFrameResource* FrameResource,
        ID3D12GraphicsCommandList* CommandList,
        const TPool<FRenderItemInfo>& RenderItems
    );
	std::unique_ptr<FCubeSkyRenderer> mSkyCubeMapRenderer;
};