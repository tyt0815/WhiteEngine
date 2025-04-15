#include "DSVHeap.h"
#include <assert.h>
#include "d3dx12.h"
#include "DXException.h"
#include "DXResourceManager.h"

FDSVHeap::FDSVHeap()
{
	mDevice = GetDXResourceManagerPtr()->GetDevicePtr();
	// DSVHeap
	D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc;
	DSVHeapDesc.NumDescriptors = DSV_NUM;
	DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSVHeapDesc.NodeMask = 0;
	THROW_IF_FAILED(
		mDevice->CreateDescriptorHeap(
			&DSVHeapDesc,
			IID_PPV_ARGS(mDSVHeap.GetAddressOf())
		)
	);

	mDSVSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

FDSVHeap::~FDSVHeap()
{
}

int FDSVHeap::CreateDepthStencilView(ID3D12Resource* Resource, const D3D12_DEPTH_STENCIL_VIEW_DESC& Desc)
{
	assert(mDSVCount < DSV_NUM);
	int DSVIndex = mDSVCount++;
	mDevice->CreateDepthStencilView(Resource, &Desc, GetDepthStencilView(DSVIndex));
	return DSVIndex;
}

D3D12_CPU_DESCRIPTOR_HANDLE FDSVHeap::GetDepthStencilView(int i) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mDSVHeap->GetCPUDescriptorHandleForHeapStart(),
		i,
		mDSVSize
	);
}
