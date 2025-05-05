#include "DepthStencil.h"
#include "DirectX/DSVHeap.h"
#include "DirectX/DXResourceManager.h"

FDepthStencil::FDepthStencil(UINT Width, UINT Height):
	mDSVHeap(GetDSVHeap()),
	FTexture::FTexture(GetDXResourceManagerPtr()->GetDevicePtr())
{
	// Build DepthStencilBuffer
	D3D12_RESOURCE_DESC DepthStencilDesc;
	ZeroMemory(&DepthStencilDesc, sizeof(D3D12_RESOURCE_DESC));
	DepthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DepthStencilDesc.Alignment = 0;
	DepthStencilDesc.Width = Width;
	DepthStencilDesc.Height = Height;
	DepthStencilDesc.DepthOrArraySize = 1;
	DepthStencilDesc.MipLevels = 1;
	DepthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	DepthStencilDesc.SampleDesc.Count = 1;
	DepthStencilDesc.SampleDesc.Quality = 0;
	DepthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DepthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE OptClear;
	OptClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	OptClear.DepthStencil.Depth = 1.0f;
	OptClear.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES DefaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CreateCommittedResource(&DefaultHeapProperties, D3D12_HEAP_FLAG_NONE, &DepthStencilDesc, &OptClear);

	// Build SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	SRVDesc.Texture2D.MipLevels = 1;
	SRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	CreateTexture2DSRV(SRVDesc);

	// Build DSV
	D3D12_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc;
	ZeroMemory(&DepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	DepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	DepthStencilViewDesc.Texture2D.MipSlice = 0;
	mDSVIndex = mDSVHeap->CreateDepthStencilView(mResource.Get(), DepthStencilViewDesc);
}

void FDepthStencil::Clear(ID3D12GraphicsCommandList* CommandList)
{
	CommandList->ClearDepthStencilView(
		GetDSV(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f,
		0,
		0,
		nullptr
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE FDepthStencil::GetDSV() const
{
	return mDSVHeap->GetDepthStencilView(mDSVIndex);
}
