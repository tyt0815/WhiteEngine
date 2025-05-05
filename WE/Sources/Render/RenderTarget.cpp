#include "RenderTarget.h"
#include <DirectXColors.h>
#include "DirectX/DXException.h"
#include "DirectX/DXResourceManager.h"
#include "DirectX/RTVHeap.h"

FRenderTarget::FRenderTarget(UINT Width, UINT Height, UINT MipLevels, DXGI_FORMAT Format):
	mRTVHeap(GetRTVHeap()),
	FTexture::FTexture(GetDXResourceManagerPtr()->GetDevicePtr())
{
	// BuildResource
	D3D12_RESOURCE_DESC TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D12_RESOURCE_DESC));
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	TextureDesc.Alignment = 0;
	TextureDesc.Width = Width;
	TextureDesc.Height = Height;
	TextureDesc.DepthOrArraySize = 1;
	TextureDesc.MipLevels = MipLevels;
	TextureDesc.Format = Format;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE OptClear;
	OptClear.Format = Format;
	OptClear.Color[0] = 0.0f;
	OptClear.Color[1] = 0.0f;
	OptClear.Color[2] = 0.0f;
	OptClear.Color[3] = 1.0f;

	D3D12_HEAP_PROPERTIES DefaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CreateCommittedResource(&DefaultHeapProperties, D3D12_HEAP_FLAG_NONE, &TextureDesc, &OptClear);

	// BuildSRV
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	SRVDesc.Texture2D.MipLevels = MipLevels;
	SRVDesc.Format = Format;
	CreateTexture2DSRV(SRVDesc);

	// Build RTV
	mRTVHeapIndices.resize(MipLevels);
	for (int i = 0; i < static_cast<int>(MipLevels); ++i)
	{
		CreateRenderTargetView(i);
	}
}

void FRenderTarget::Clear(ID3D12GraphicsCommandList* CommandList, int MipLevel)
{
	CommandList->ClearRenderTargetView(
		GetRTV(MipLevel),
		DirectX::Colors::Black,
		0,
		nullptr
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE FRenderTarget::GetRTV(int MipLevel)
{
	return mRTVHeap->GetCPURTV(mRTVHeapIndices[MipLevel]);
}

void FRenderTarget::CreateRenderTargetView(int MipLevel)
{
	D3D12_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Format = mResource->GetDesc().Format;
	RTVDesc.Texture2D.PlaneSlice = 0;
	RTVDesc.Texture2D.MipSlice = MipLevel;
	mRTVHeapIndices[MipLevel] = mRTVHeap->CreateRenderTargetView(mResource.Get(), RTVDesc);
}