#include "CBVSRVUAVHeap.h"
#include <assert.h>
#include "d3dx12.h"
#include "DXResourceManager.h"

FCBVSRVUAVHeap::FCBVSRVUAVHeap():
	mDevice(GetDXResourceManagerPtr()->GetDevicePtr())
{
	D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc;
	ZeroMemory(&SRVHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	SRVHeapDesc.NumDescriptors = TEXTURE2D_VIEW_NUM + TEXTURECUBE_VIEW_NUM;
	SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	THROW_IF_FAILED(
		mDevice->CreateDescriptorHeap(
			&SRVHeapDesc,
			IID_PPV_ARGS(mCBVSRVUAVHeap.GetAddressOf())
		)
	);
	mDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

FCBVSRVUAVHeap::~FCBVSRVUAVHeap()
{
}

int FCBVSRVUAVHeap::CreateEmptyTexture2DSRV()
{
	assert(mTexture2DViewCount < TEXTURE2D_VIEW_NUM);
	int SRVHeapIndex = mTexture2DViewCount++;
	return SRVHeapIndex;
}

int FCBVSRVUAVHeap::CreateTexture2DSRV(ID3D12Resource* Resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc)
{
	assert(mTexture2DViewCount < TEXTURE2D_VIEW_NUM);
	int SRVHeapIndex = mTexture2DViewCount++;
	mDevice->CreateShaderResourceView(Resource, &SRVDesc, GetTexture2DCPUDescriptorHandle(SRVHeapIndex));
	return SRVHeapIndex;
}

int FCBVSRVUAVHeap::CreateTexture2DUAV(ID3D12Resource* Resource, ID3D12Resource* CounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc)
{
	assert(mTexture2DViewCount < TEXTURE2D_VIEW_NUM);
	int UAVIndex = mTexture2DViewCount++;
	mDevice->CreateUnorderedAccessView(Resource, CounterResource,&Desc, GetTexture2DCPUDescriptorHandle(UAVIndex));
	return UAVIndex;
}

int FCBVSRVUAVHeap::CreateTextureCubeSRV(ID3D12Resource* Resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& SRVDesc)
{
	assert(mTextureCubeViewCount < TEXTURECUBE_VIEW_NUM);
	int SRVHeapIndex = mTextureCubeViewCount++;
	mDevice->CreateShaderResourceView(Resource, &SRVDesc, GetTextureCubeCPUDescriptorHandle(SRVHeapIndex));
	return SRVHeapIndex;
}

D3D12_CPU_DESCRIPTOR_HANDLE FCBVSRVUAVHeap::GetTexture2DCPUDescriptorHandleStart() const
{
	return mCBVSRVUAVHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE FCBVSRVUAVHeap::GetTexture2DGPUDescriptorHandleStart() const
{
	return mCBVSRVUAVHeap->GetGPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE FCBVSRVUAVHeap::GetTexture2DCPUDescriptorHandle(int i) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		GetTexture2DCPUDescriptorHandleStart(),
		i,
		mDescriptorSize
	);
}

D3D12_GPU_DESCRIPTOR_HANDLE FCBVSRVUAVHeap::GetTexture2DGPUDescriptorHandle(int i) const
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(
		GetTexture2DGPUDescriptorHandleStart(),
		i,
		mDescriptorSize
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE FCBVSRVUAVHeap::GetTextureCubeCPUDescriptorHandleStart() const
{
	return GetTexture2DCPUDescriptorHandle(TEXTURE2D_VIEW_NUM);
}

D3D12_GPU_DESCRIPTOR_HANDLE FCBVSRVUAVHeap::GetTextureCubeGPUDescriptorHandleStart() const
{
	return GetTexture2DGPUDescriptorHandle(TEXTURE2D_VIEW_NUM);
}

D3D12_CPU_DESCRIPTOR_HANDLE FCBVSRVUAVHeap::GetTextureCubeCPUDescriptorHandle(int i) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		GetTextureCubeCPUDescriptorHandleStart(),
		i,
		mDescriptorSize
	);
}

D3D12_GPU_DESCRIPTOR_HANDLE FCBVSRVUAVHeap::GetTextureCubeGPUDescriptorHandle(int i) const
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(
		GetTextureCubeGPUDescriptorHandleStart(),
		i,
		mDescriptorSize
	);
}
