//***************************************************************************************
// FCubeRenderTarget.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "CubeRenderTarget.h"
#include <memory>
#include <cassert>
#include "DirectX/DXResourceManager.h"
#include "Texture.h"

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

FCubeRenderTarget::FCubeRenderTarget(
	std::string Name,
	UINT Width,
	UINT Height,
	UINT MipLevels,
	DXGI_FORMAT Format,
	DXGI_FORMAT DepthStencilFormat
):
	mName(Name),
	mWidth(Width),
	mHeight(Height),
	mMipLevels(MipLevels),
	mFormat(Format),
	mDepthStencilFormat(DepthStencilFormat),
	mViewport({ 0.0f, 0.0f, (float)Width, (float)Height, 0.0f, 1.0f }),
	mScissorRect({ 0, 0, (int)Width, (int)Height })
{
	Initialize();
}

D3D12_CPU_DESCRIPTOR_HANDLE FCubeRenderTarget::GetRTV(int FaceIndex, int MipLevel) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mRTVHeap->GetCPUDescriptorHandleForHeapStart(),
		FaceIndex * mMipLevels + MipLevel,
		GetDXResourceManagerPtr()->GetRTVDescriptorSize()
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE FCubeRenderTarget::GetCubeMapCPUDescriptorHeap() const
{
	return GetTextureManager()->GetTextureCubeCPUDescriptorHandle(mTexture->SRVHeapIndex);
}

D3D12_GPU_DESCRIPTOR_HANDLE FCubeRenderTarget::GetCubeMapGPUDescriptorHeap() const
{
	return GetTextureManager()->GetTextureCubeGPUDescriptorHandle(mTexture->SRVHeapIndex);
}

D3D12_VIEWPORT FCubeRenderTarget::GetViewportMipLevel(int i) const
{
	D3D12_VIEWPORT Viewport = GetViewport();
	Viewport.Width = static_cast<float>(max(1.0f, Viewport.Width / pow(2, i)));
	Viewport.Height = static_cast<float>(max(1.0f, Viewport.Height / pow(2, i)));
	return Viewport;
}

D3D12_RECT FCubeRenderTarget::GetScissorRectMipLevel(int i) const
{
	D3D12_RECT ScissorRect = GetScissorRect();
	ScissorRect.right = static_cast<int>(max(1, ScissorRect.right / pow(2, i)));
	ScissorRect.bottom = static_cast<int>(max(1, ScissorRect.bottom / pow(2, i)));
	return ScissorRect;
}

FCubeRenderTarget::~FCubeRenderTarget()
{
	mRTVHeap = nullptr;
	mDSVHeap = nullptr;
	mCubeMapResource = nullptr;
	mDepthStencilResource = nullptr;
}

void FCubeRenderTarget::OnResize(UINT Width, UINT Height)
{
	if ((mWidth != Width) || (mHeight != Height))
	{
		mWidth = Width;
		mHeight = Height;

		Initialize();
	}
}

void FCubeRenderTarget::Initialize()
{
	BuildResource();
	BuildRTVAndDSV();
	BuildDescriptors();
}

void FCubeRenderTarget::BuildRTVAndDSV()
{
	FDXResourceManager* DXManager = GetDXResourceManagerPtr();
	ID3D12Device* Device = DXManager->GetDevicePtr();
	// RTVHeap
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc;
	RTVHeapDesc.NumDescriptors = 6 * mMipLevels;
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTVHeapDesc.NodeMask = 0;
	THROW_IF_FAILED(
		Device->CreateDescriptorHeap(
			&RTVHeapDesc,
			IID_PPV_ARGS(mRTVHeap.GetAddressOf())
		)
	);

	// DSVHeap
	D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc;
	DSVHeapDesc.NumDescriptors = 1;
	DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSVHeapDesc.NodeMask = 0;
	THROW_IF_FAILED(
		Device->CreateDescriptorHeap(
			&DSVHeapDesc,
			IID_PPV_ARGS(mDSVHeap.GetAddressOf())
		)
	);

	Device->CreateDepthStencilView(
		mDepthStencilResource.Get(),
		nullptr,
		mDSVHeap->GetCPUDescriptorHandleForHeapStart()
	);
}

void FCubeRenderTarget::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = mFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = mMipLevels;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

	// Create SRV to the entire cubemap resource.
	FDXResourceManager* DXManager = GetDXResourceManagerPtr();
	ID3D12Device* Device = DXManager->GetDevicePtr();
	Device->CreateShaderResourceView(mCubeMapResource.Get(), &srvDesc, GetCubeMapCPUDescriptorHeap());

	// Create RTV to each cube face.
	for (int i = 0; i < 6; ++i)
	{
		D3D12_RENDER_TARGET_VIEW_DESC RTVDesc;
		RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		RTVDesc.Format = mFormat;
		RTVDesc.Texture2DArray.PlaneSlice = 0;
		RTVDesc.Texture2DArray.FirstArraySlice = i;
		RTVDesc.Texture2DArray.ArraySize = 1;
		for (UINT j = 0; j < mMipLevels; ++j)
		{
			// Render target to ith element, jth mipmap.
			RTVDesc.Texture2DArray.MipSlice = j;
			Device->CreateRenderTargetView(mCubeMapResource.Get(), &RTVDesc, GetRTV(i, j));
		}
	}
}

void FCubeRenderTarget::BuildResource()
{
	// Note, compressed formats cannot be used for UAV.  We get error like:
	// ERROR: ID3D11Device::CreateTexture2D: The format (0x4d, BC3_UNORM) 
	// cannot be bound as an UnorderedAccessView, or cast to a format that
	// could be bound as an UnorderedAccessView.  Therefore this format 
	// does not support D3D11_BIND_UNORDERED_ACCESS.

	D3D12_RESOURCE_DESC TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D12_RESOURCE_DESC));
	TextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	TextureDesc.Alignment = 0;
	TextureDesc.Width = mWidth;
	TextureDesc.Height = mHeight;
	TextureDesc.DepthOrArraySize = 6;
	TextureDesc.MipLevels = mMipLevels;
	TextureDesc.Format = mFormat;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	TextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE OptClear;
	ZeroMemory(&OptClear, sizeof(D3D12_CLEAR_VALUE));
	OptClear.Format = mFormat;
	OptClear.Color[3] = 1.0f;

	FDXResourceManager* DXManager = GetDXResourceManagerPtr();
	ID3D12Device* Device = DXManager->GetDevicePtr();
	D3D12_HEAP_PROPERTIES DefaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	THROW_IF_FAILED(
		Device->CreateCommittedResource(
			&DefaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&TextureDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			&OptClear,
			IID_PPV_ARGS(&mCubeMapResource)
		)
	);
	FTextureManager* TexManager = GetTextureManager();
	std::unique_ptr<FTexture> Texture = std::make_unique<FTexture>();
	mTexture = Texture.get();
	Texture->Name = mName;
	Texture->Resource = mCubeMapResource;
	TexManager->RegisterTextureCube(std::move(Texture));

	// Build Depth Stencil Buffer
	D3D12_RESOURCE_DESC DepthStencilDesc;
	ZeroMemory(&DepthStencilDesc, sizeof(D3D12_RESOURCE_DESC));
	DepthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	DepthStencilDesc.Alignment = 0;
	DepthStencilDesc.Width = mWidth;
	DepthStencilDesc.Height = mHeight;
	DepthStencilDesc.DepthOrArraySize = 1;
	DepthStencilDesc.MipLevels = 1;
	DepthStencilDesc.Format = mDepthStencilFormat;
	DepthStencilDesc.SampleDesc.Count = 1;
	DepthStencilDesc.SampleDesc.Quality = 0;
	DepthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	DepthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	OptClear.Format = mDepthStencilFormat;
	OptClear.DepthStencil.Depth = 1.0f;
	OptClear.DepthStencil.Stencil = 0;
	THROW_IF_FAILED(
		Device->CreateCommittedResource(
			&DefaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&DepthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&OptClear,
			IID_PPV_ARGS(mDepthStencilResource.GetAddressOf())
		)
	);
}