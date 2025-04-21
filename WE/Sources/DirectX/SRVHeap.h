#pragma once

#include <d3d12.h>
#include <wrl.h>
#include "Utility/Class.h"

constexpr int TEXTURE2D_NUM = 1024;
constexpr int TEXTURECUBE_NUM = 1024;

class FSRVHeap final
{
	SINGLETON(FSRVHeap);
public:
	int CreateTexture2DSRV(ID3D12Resource* Resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc);
	int CreateTextureCubeSRV(ID3D12Resource* Resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc);
	D3D12_CPU_DESCRIPTOR_HANDLE GetTexture2DCPUSRVStart() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetTexture2DGPUSRVStart() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetTexture2DCPUDescriptorHandle(int i) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetTexture2DGPUDescriptorHandle(int i) const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureCubeCPUSRVStart() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureCubeGPUSRVStart() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureCubeCPUDescriptorHandle(int i) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureCubeGPUDescriptorHandle(int i) const;

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSRVHeap;
	ID3D12Device* mDevice;
	UINT mDescriptorSize = 0;
	int mTexture2DCount = 0;
	int mTextureCubeCount = 0;
public:
	inline ID3D12DescriptorHeap* Get()
	{
		return mSRVHeap.Get();
	}
};

inline FSRVHeap* GetSRVHeap()
{
	return FSRVHeap::GetInstance();
}