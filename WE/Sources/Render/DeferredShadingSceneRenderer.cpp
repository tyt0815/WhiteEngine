#include "DeferredShadingSceneRenderer.h"
#include "DirectX/DXResourceManager.h"

void FDeferredShadingSceneRenderer::Render(const FRenderingData& RenderingData)
{
	Super::Render(RenderingData);

	ID3D12Device* Device = GetDXResourceManagerPtr()->GetDevicePtr();
	
}

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
