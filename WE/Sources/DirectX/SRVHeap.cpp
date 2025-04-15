#include "SRVHeap.h"
#include <assert.h>
#include "d3dx12.h"
#include "DXResourceManager.h"

FSRVHeap::FSRVHeap():
	mDevice(GetDXResourceManagerPtr()->GetDevicePtr())
{
	D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc;
	ZeroMemory(&SRVHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	SRVHeapDesc.NumDescriptors = TEXTURE2D_NUM + TEXTURECUBE_NUM;
	SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	THROW_IF_FAILED(
		mDevice->CreateDescriptorHeap(
			&SRVHeapDesc,
			IID_PPV_ARGS(mSRVHeap.GetAddressOf())
		)
	);
	mDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

FSRVHeap::~FSRVHeap()
{
}

int FSRVHeap::CreateTexture2DSRV(ID3D12Resource* Resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc)
{
	assert(mTexture2DCount < TEXTURE2D_NUM);
	int SRVHeapIndex = mTexture2DCount++;
	mDevice->CreateShaderResourceView(Resource, &SRVDesc, GetCPUDescriptorHandle(SRVHeapIndex));
	return SRVHeapIndex;
}

int FSRVHeap::CreateTextureCubeSRV(ID3D12Resource* Resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc)
{
	assert(mTextureCubeCount < TEXTURECUBE_NUM);
	int SRVHeapIndex = TEXTURE2D_NUM + mTextureCubeCount++;
	mDevice->CreateShaderResourceView(Resource, &SRVDesc, GetCPUDescriptorHandle(SRVHeapIndex));
	return SRVHeapIndex;
}

D3D12_GPU_DESCRIPTOR_HANDLE FSRVHeap::GetTexture2DSRVStart() const
{
	return mSRVHeap->GetGPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE FSRVHeap::GetTextureCubeSRVStart() const
{
	return GetGPUDescriptorHandle(TEXTURE2D_NUM);
}

D3D12_CPU_DESCRIPTOR_HANDLE FSRVHeap::GetCPUDescriptorHandle(int i) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mSRVHeap->GetCPUDescriptorHandleForHeapStart(),
		i,
		mDescriptorSize
	);
}

D3D12_GPU_DESCRIPTOR_HANDLE FSRVHeap::GetGPUDescriptorHandle(int i) const
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(
		mSRVHeap->GetGPUDescriptorHandleForHeapStart(),
		i,
		mDescriptorSize
	);
}