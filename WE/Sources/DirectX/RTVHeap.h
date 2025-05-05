#pragma once

#include <d3d12.h>
#include <wrl.h>
#include "Utility/Class.h"

constexpr UINT RTV_NUM = 1024;

class FRTVHeap
{
	SINGLETON(FRTVHeap);
public:
	int CreateRenderTargetView(ID3D12Resource* Resource, const D3D12_RENDER_TARGET_VIEW_DESC& Desc);
	void CreateRenderTargetView(ID3D12Resource* Resource, const D3D12_RENDER_TARGET_VIEW_DESC& Desc, D3D12_CPU_DESCRIPTOR_HANDLE RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPURTV(int i) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPURTV(int i) const;

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRTVHeap;
	ID3D12Device* mDevice;
	UINT mRTVSize = 0;
	int mRTVCount = 0;

public:
	inline int NextRTVIndex()
	{
		return mRTVCount++;
	}
};

inline FRTVHeap* GetRTVHeap()
{
	return FRTVHeap::GetInstance();
}