#include "UnorderedAccessTexture2D.h"
#include "DirectX/CBVSRVUAVHeap.h"

FUnorderedAccessTexture2D::FUnorderedAccessTexture2D(ID3D12Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format) :
	FTexture::FTexture(Device)
{
	// BuildResource
	D3D12_RESOURCE_DESC TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D12_RESOURCE_DESC));
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	TextureDesc.Alignment = 0;
	TextureDesc.Width = Width;
	TextureDesc.Height = Height;
	TextureDesc.DepthOrArraySize = 1;
	TextureDesc.MipLevels = 1;
	TextureDesc.Format = Format;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_HEAP_PROPERTIES DefaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CreateCommittedResource(&DefaultHeapProperties, D3D12_HEAP_FLAG_NONE, &TextureDesc, nullptr);

	// BuildSRV
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	SRVDesc.Texture2D.MipLevels = 1;
	SRVDesc.Format = Format;
	CreateTexture2DSRV(SRVDesc);

	D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
	ZeroMemory(&UAVDesc, sizeof(D3D12_UNORDERED_ACCESS_VIEW_DESC));
	UAVDesc.Format = Format;
	UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	CreateTexture2DUAV(UAVDesc);
}

void FUnorderedAccessTexture2D::CreateTexture2DUAV(const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc)
{
	mUAVIndex = mCBVSRVUAVHeap->CreateTexture2DUAV(mResource.Get(), nullptr, Desc);
}
