#include "Resource.h"
#include "d3dx12.h"

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
