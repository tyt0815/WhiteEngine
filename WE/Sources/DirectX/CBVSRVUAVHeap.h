#pragma once

#include <d3d12.h>
#include <wrl.h>
#include "Utility/Class.h"

constexpr int TEXTURE2D_VIEW_NUM = 1024;
constexpr int TEXTURECUBE_VIEW_NUM = 1024;

class FCBVSRVUAVHeap final
{
	SINGLETON(FCBVSRVUAVHeap);
public:
	int CreateTexture2DSRV(ID3D12Resource* Resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc);
	int CreateTextureCubeSRV(ID3D12Resource* Resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc);
	D3D12_CPU_DESCRIPTOR_HANDLE GetTexture2DCPUDescriptorHandleStart() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetTexture2DGPUDescriptorHandleStart() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetTexture2DCPUDescriptorHandle(int i) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetTexture2DGPUDescriptorHandle(int i) const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureCubeCPUDescriptorHandleStart() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureCubeGPUDescriptorHandleStart() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureCubeCPUDescriptorHandle(int i) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureCubeGPUDescriptorHandle(int i) const;

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCBVSRVUAVHeap;
	ID3D12Device* mDevice;
	UINT mDescriptorSize = 0;
	int mTexture2DViewCount = 0;
	int mTextureCubeViewCount = 0;
public:
	inline ID3D12DescriptorHeap* Get()
	{
		return mCBVSRVUAVHeap.Get();
	}
};

inline FCBVSRVUAVHeap* GetCBVSRVUAVHeap()
{
	return FCBVSRVUAVHeap::GetInstance();
}