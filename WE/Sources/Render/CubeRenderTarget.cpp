#include "CubeRenderTarget.h"
#include <DirectXColors.h>
#include "DirectX/DXResourceManager.h"
#include "DirectX/DXMath.h"
#include "DirectX/RTVHeap.h"

std::array<DirectX::XMFLOAT4X4, 6> FCubeRenderTarget::GetCubeMapViews()
{
	// Build ViewMat
	// Generate the cube map about the given position.
	XMFLOAT3 Position(0.0f, 0.0f, 0.0f);

	// Look along each coordinate axis.
	XMFLOAT3 targets[6] =
	{
		XMFLOAT3(1.0f, 0.0f, 0.0f), // +X
		XMFLOAT3(-1.0f, 0.0f, 0.0f), // -X
		XMFLOAT3(0.0f, 1.0f, 0.0f), // +Y
		XMFLOAT3(0.0f, -1.0f, 0.0f), // -Y
		XMFLOAT3(0.0f, 0.0f, 1.0f), // +Z
		XMFLOAT3(0.0f, 0.0f, -1.0f)  // -Z
	};

	// Use world up vector (0,1,0) for all directions except +Y/-Y.  In these cases, we
	// are looking down +Y or -Y, so we need a different "up" vector.
	XMFLOAT3 ups[6] =
	{
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // +X
		XMFLOAT3(0.0f, 1.0f, 0.0f),  // -X
		XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y
		XMFLOAT3(0.0f, 0.0f, +1.0f), // -Y
		XMFLOAT3(0.0f, 1.0f, 0.0f),	 // +Z
		XMFLOAT3(0.0f, 1.0f, 0.0f)	 // -Z
	};

	return {
		FDXMath::CalcViewMatrix(targets[0], ups[0], Position),
		FDXMath::CalcViewMatrix(targets[1], ups[1], Position),
		FDXMath::CalcViewMatrix(targets[2], ups[2], Position),
		FDXMath::CalcViewMatrix(targets[3], ups[3], Position),
		FDXMath::CalcViewMatrix(targets[4], ups[4], Position),
		FDXMath::CalcViewMatrix(targets[5], ups[5], Position)
	};
}

FCubeRenderTarget::FCubeRenderTarget(UINT Width, UINT Height, UINT MipLevels, DXGI_FORMAT Format):
	mRTVHeap(GetRTVHeap()),
	FTexture::FTexture(GetDXResourceManagerPtr()->GetDevicePtr())
{
	// Build Resource
	D3D12_RESOURCE_DESC TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D12_RESOURCE_DESC));
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	TextureDesc.Alignment = 0;
	TextureDesc.Width = Width;
	TextureDesc.Height = Height;
	TextureDesc.DepthOrArraySize = 6;
	TextureDesc.MipLevels = MipLevels;
	TextureDesc.Format = Format;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE OptClear;
	ZeroMemory(&OptClear, sizeof(D3D12_CLEAR_VALUE));
	OptClear.Format = Format;
	OptClear.Color[3] = 1.0f;

	D3D12_HEAP_PROPERTIES DefaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CreateCommittedResource(&DefaultHeapProperties, D3D12_HEAP_FLAG_NONE, &TextureDesc, &OptClear);

	// Build SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.Format = Format;
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	SRVDesc.TextureCube.MostDetailedMip = 0;
	SRVDesc.TextureCube.MipLevels = MipLevels;
	SRVDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	CreateTextureCubeSRV(SRVDesc);

	// Build RTV
	D3D12_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	RTVDesc.Format = Format;
	RTVDesc.Texture2DArray.PlaneSlice = 0;
	RTVDesc.Texture2DArray.ArraySize = 1;
	mRTVHeapIndices.resize(6);
	for (int i = 0; i < 6; ++i)
	{
		RTVDesc.Texture2DArray.FirstArraySlice = i;
		mRTVHeapIndices[i].resize(MipLevels);
		for (int j = 0; j < static_cast<int>(MipLevels); ++j)
		{
			RTVDesc.Texture2DArray.MipSlice = j;
			mRTVHeapIndices[i][j] = mRTVHeap->CreateRenderTargetView(mResource.Get(), RTVDesc);
		}
	}
}

void FCubeRenderTarget::Clear(ID3D12GraphicsCommandList* CommandList, int FaceIndex, int MipLevel)
{
	D3D12_RECT ScissorRect = GetScissorRect();
	CommandList->ClearRenderTargetView(
		GetRTV(FaceIndex, MipLevel),
		DirectX::Colors::Black,
		1,
		&ScissorRect
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE FCubeRenderTarget::GetRTV(int FaceIndex, int MipLevel)
{
	return mRTVHeap->GetCPURTV(mRTVHeapIndices[FaceIndex][MipLevel]);
}
