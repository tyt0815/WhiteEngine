#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include "Utility/Class.h"

class FResource final : FNoncopyable
{
public:
	FResource() = delete;
	void TransitResourceBarrier(ID3D12GraphicsCommandList* CommandList, D3D12_RESOURCE_STATES ResourceState);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
	D3D12_RESOURCE_STATES mResourceState;

public:
	inline ID3D12Resource* Get() const
	{
		return mResource.Get();
	}
};