#pragma once

#include <d3d12.h>
#include <wrl.h>
#include "Utility/Class.h"

constexpr UINT DSV_NUM = 1024;

class FDSVHeap
{
	SINGLETON(FDSVHeap);
public:
	int CreateDepthStencilView(ID3D12Resource* Resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView(int i) const;

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDSVHeap;
	ID3D12Device* mDevice;

	int mDSVCount = 0;
	UINT mDSVSize = 0;
};

inline FDSVHeap* GetDSVHeap()
{
	return FDSVHeap::GetInstance();
}