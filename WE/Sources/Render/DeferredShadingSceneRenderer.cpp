#include "DeferredShadingSceneRenderer.h"
#include "DirectX/DXResourceManager.h"

void FDeferredShadingSceneRenderer::Render()
{
	Super::Render();

	ID3D12Device* Device = GetDXResourceManagerPtr()->GetDevicePtr();
	
}

void FDeferredShadingSceneRenderer::BuildShadersAndInputLayouts()
{
}

void FDeferredShadingSceneRenderer::BuildPipelineStates()
{
}

void FDeferredShadingSceneRenderer::CreateFrameResources()
{
	Super::CreateFrameResources();
}

void FDeferredShadingSceneRenderer::BuildRootSignature()
{
	Super::BuildRootSignature();
}

void FDeferredShadingSceneRenderer::UpdateFrameBuffers(FFrameResource* FrameResource)
{
	Super::UpdateFrameBuffers(FrameResource);
}
