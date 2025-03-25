#pragma once

#include <d3d12.h>
#include <unordered_map>
#include <memory>
#include <vector>

#include "GameFramework/Object/Actor/Actor.h"
#include "Utility/Class.h"
#include "FrameResource.h"
#include "RenderItemManager.h"

class FCubeSkyRenderer;

struct FRenderData
{
	FFrameResource* FrameResource;
};

class FRenderer : FNoncopyable
{
	enum class EPipelineState
	{
		EPS_Opaque,
		EPS_WireFrame,
		EPS_Transparency,
		EPS_AlphaTest,
		EPS_Billboard,
		EPS_None
	};
public:
	FRenderer();
	virtual void Render(const FRenderData& RenderData);

private:
	void DrawRenderItems(FFrameResource* FrameResource, ID3D12GraphicsCommandList* CommandList, const TPool<FRenderItemInfo>& RenderItems);
	std::unique_ptr<FCubeSkyRenderer> mSkyCubeMapRenderer;
	bool bWireFrame = false;
};