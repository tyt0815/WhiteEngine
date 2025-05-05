#pragma once

#include <d3d12.h>
#include <wrl.h>
#include "Utility/Class.h"

class FResource : FNoncopyable
{
public:
	FResource(ID3D12Device* Device);
	virtual ~FResource() {};
	void TransitResourceBarrier(ID3D12GraphicsCommandList* CommandList, D3D12_RESOURCE_STATES ResourceState);
	void CreateCommittedResource(
		D3D12_HEAP_PROPERTIES* HeapProperties,
		const D3D12_HEAP_FLAGS& HeapFlags,
		D3D12_RESOURCE_DESC* ResourceDesc,
		D3D12_CLEAR_VALUE* ClearValue
	);
	D3D12_VIEWPORT GetViewport() const;
	D3D12_RECT GetScissorRect() const;
	D3D12_VIEWPORT GetViewportMipLevel(int i) const;
	D3D12_RECT GetScissorRectMipLevel(int i) const;

protected:
	FResource() {};
	Microsoft::WRL::ComPtr<ID3D12Resource> mResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mUploadHeap = nullptr;
	ID3D12Device* mDevice = nullptr;

private:
	D3D12_RESOURCE_STATES mResourceState = D3D12_RESOURCE_STATE_COMMON;

public:
	inline ID3D12Resource* Get() const
	{
		return mResource.Get();
	}
	inline ID3D12Resource* * GetAddressOf() 
	{
		return mResource.GetAddressOf();
	}
	inline D3D12_RESOURCE_DESC GetDesc() const
	{
		return mResource->GetDesc();
	}
	inline UINT64 GetWidth() const
	{
		return GetDesc().Width;
	}
	inline UINT64 GetHeight() const
	{
		return GetDesc().Height;
	}
	inline virtual DXGI_FORMAT GetFormat() const
	{
		return GetDesc().Format;
	}
	inline UINT16 GetMipLevels() const
	{
		return GetDesc().MipLevels;
	}
	inline void Reset() 
	{
		mResource.Reset();
		mUploadHeap.Reset();
	}
};