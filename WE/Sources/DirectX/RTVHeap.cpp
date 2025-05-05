#include "RTVHeap.h"
#include "DXResourceManager.h"

FRTVHeap::FRTVHeap():
	mDevice(GetDXResourceManagerPtr()->GetDevicePtr())
{
	// DSVHeap
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc;
	RTVHeapDesc.NumDescriptors = RTV_NUM;
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTVHeapDesc.NodeMask = 0;
	THROW_IF_FAILED(
		mDevice->CreateDescriptorHeap(
			&RTVHeapDesc,
			IID_PPV_ARGS(mRTVHeap.GetAddressOf())
		)
	);

	mRTVSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}
FRTVHeap::~FRTVHeap()
{

}

int FRTVHeap::CreateRenderTargetView(ID3D12Resource* Resource, const D3D12_RENDER_TARGET_VIEW_DESC& Desc)
{
	const int RTVHeapIndex = NextRTVIndex();
	CreateRenderTargetView(Resource, Desc, GetCPURTV(RTVHeapIndex));
	return RTVHeapIndex;
}

void FRTVHeap::CreateRenderTargetView(ID3D12Resource* Resource, const D3D12_RENDER_TARGET_VIEW_DESC& Desc, D3D12_CPU_DESCRIPTOR_HANDLE RTV)
{
	mDevice->CreateRenderTargetView(Resource, &Desc, RTV);
}

D3D12_CPU_DESCRIPTOR_HANDLE FRTVHeap::GetCPURTV(int i) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mRTVHeap->GetCPUDescriptorHandleForHeapStart(),
		i,
		mRTVSize
	);
}

D3D12_GPU_DESCRIPTOR_HANDLE FRTVHeap::GetGPURTV(int i) const
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(
		mRTVHeap->GetGPUDescriptorHandleForHeapStart(),
		i,
		mRTVSize
	);
}
