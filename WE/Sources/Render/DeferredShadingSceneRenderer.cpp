#include "DeferredShadingSceneRenderer.h"
#include "DirectX/DXResourceManager.h"

void FDeferredShadingSceneRenderer::BuildShadersAndInputLayouts()
{
}

void FDeferredShadingSceneRenderer::BuildPipelineStates(ID3D12Device* Device)
{
}

void FDeferredShadingSceneRenderer::CreateFrameResources(ID3D12Device* Device)
{
}

void FDeferredShadingSceneRenderer::BuildRootSignature()
{
}

void FDeferredShadingSceneRenderer::UpdateFrameBuffers(FFrameResourceBase* FrameResource)
{
	Super::UpdateFrameBuffers(FrameResource);
}

void FDeferredShadingSceneRenderer::Render(ID3D12GraphicsCommandList* CommandList, FFrameResourceBase* FrameResource)
{
}
