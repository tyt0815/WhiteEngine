#include "SceneRenderer.h"
#include "DirectX/DXResourceManager.h"


const int gFrameResourcesNum = FRAME_RESOURCES_NUM;

FFrameResource::FFrameResource()
{
	ID3D12Device* Device = GetDXResourceManagerPtr()->GetDevicePtr();
	THROW_IF_FAILED(
		Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(mCommandAllocator.GetAddressOf())
		)
	);
}

FFrameResource::~FFrameResource()
{
}

void FFrameResource::Flush()
{
	ID3D12Fence* Fence = GetDXResourceManagerPtr()->GetFencePtr();
	if (mFenceCount != 0 && Fence->GetCompletedValue() < mFenceCount)
	{
		HANDLE EventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		THROW_IF_FAILED(Fence->SetEventOnCompletion(mFenceCount, EventHandle));
		if (EventHandle)
		{
			WaitForSingleObject(EventHandle, INFINITE);
			CloseHandle(EventHandle);
		}
	}
}


FSceneRenderer::FSceneRenderer()
{
}

void FSceneRenderer::Render()
{
	UpdateTargetFrameResource();
	
}

void FSceneRenderer::Destroy()
{
	// ¸ðµç CommandAllocator¸¦ Flush
	SwitchToNextFrameResource();
	SwitchToNextFrameResource();
	SwitchToNextFrameResource();
}

void FSceneRenderer::UpdateTargetFrameResource()
{
	SwitchToNextFrameResource();
	UpdateFrameBuffers(GetTargetFrameResource());
}

void FSceneRenderer::SwitchToNextFrameResource()
{
	FDXResourceManager* DXResourceManager = GetDXResourceManagerPtr();
	DXResourceManager->SignalFence();
	GetTargetFrameResource()->SetFenceCount(DXResourceManager->GetCurrentFence());
	mTargetFrameResourceIndex = (mTargetFrameResourceIndex + 1) % gFrameResourcesNum;
	GetTargetFrameResource()->Flush();
}
