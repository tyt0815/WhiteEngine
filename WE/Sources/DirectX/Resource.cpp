#include "Resource.h"
#include "d3dx12.h"
#include "DXResourceManager.h"
#include "CBVSRVUAVHeap.h"

void FResource::TransitResourceBarrier(ID3D12GraphicsCommandList* CommandList, D3D12_RESOURCE_STATES ResourceState)
{
	if (ResourceState != mResourceState)
	{
		D3D12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			mResource.Get(),
			mResourceState,
			ResourceState
		);
		CommandList->ResourceBarrier(1, &ResourceBarrier);
		mResourceState = ResourceState;
	}
}

D3D12_VIEWPORT FResource::GetViewport() const
{
	D3D12_VIEWPORT Viewport;
	ZeroMemory(&Viewport, sizeof(D3D12_VIEWPORT));
	D3D12_RESOURCE_DESC Desc = GetDesc();
	Viewport.Width = static_cast<float>(Desc.Width);
	Viewport.Height = static_cast<float>(Desc.Height);
	Viewport.MaxDepth = 1.0f; 
	return Viewport;
}

D3D12_RECT FResource::GetScissorRect() const
{
	D3D12_RECT ScissorRect;
	ZeroMemory(&ScissorRect, sizeof(D3D12_RECT));
	D3D12_RESOURCE_DESC Desc = GetDesc();
	ScissorRect.right = static_cast<LONG>(Desc.Width);
	ScissorRect.bottom = static_cast<LONG>(Desc.Height);
	return ScissorRect;
}

D3D12_VIEWPORT FResource::GetViewportMipLevel(int i) const
{
	D3D12_VIEWPORT Viewport = GetViewport();
	Viewport.Width = static_cast<float>(max(1.0f, Viewport.Width / pow(2, i)));
	Viewport.Height = static_cast<float>(max(1.0f, Viewport.Height / pow(2, i)));
	return Viewport;
}

D3D12_RECT FResource::GetScissorRectMipLevel(int i) const
{
	D3D12_RECT ScissorRect = GetScissorRect();
	ScissorRect.right = static_cast<int>(max(1, ScissorRect.right / pow(2, i)));
	ScissorRect.bottom = static_cast<int>(max(1, ScissorRect.bottom / pow(2, i)));
	return ScissorRect;
}

void FResource::CreateCommittedResource(
	const D3D12_HEAP_PROPERTIES& HeapProperties,
	const D3D12_HEAP_FLAGS& HeapFlags,
	const D3D12_RESOURCE_DESC& ResourceDesc,
	const D3D12_CLEAR_VALUE& ClearValue
)
{
	ID3D12Device* Device = GetDXResourceManagerPtr()->GetDevicePtr();
	THROW_IF_FAILED(
		Device->CreateCommittedResource(
			&HeapProperties,
			HeapFlags,
			&ResourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&ClearValue,
			IID_PPV_ARGS(mResource.GetAddressOf())
		)
	);
	mResource->GetDesc();
}