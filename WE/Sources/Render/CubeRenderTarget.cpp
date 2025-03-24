//***************************************************************************************
// FCubeRenderTarget.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "CubeRenderTarget.h"
#include <memory>
#include <cassert>
#include "DirectX/DXResourceManager.h"
#include "Texture.h"

FCubeRenderTarget::FCubeRenderTarget(std::string Name, UINT width, UINT height, DXGI_FORMAT format, UINT MipLevels)
{
	mDevice = GetDXResourceManagerPtr()->GetDevicePtr();

	mWidth = width;
	mHeight = height;
	mFormat = format;
	mMipLevels = MipLevels;

	mViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)width, (int)height };

	BuildResource();
	std::unique_ptr<FTexture> Texture = std::make_unique<FTexture>();
	mTexture = Texture.get();
	Texture->Name = Name;
	Texture->Resource = mCubeMap;
	GetTextureManager()->RegisterTextureCube(std::move(Texture));
}

ID3D12Resource* FCubeRenderTarget::Resource()
{
	return mCubeMap.Get();
}

CD3DX12_GPU_DESCRIPTOR_HANDLE FCubeRenderTarget::Srv()
{
	return mhGpuSrv;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE FCubeRenderTarget::Rtv(int faceIndex, int MipLevel)
{
	return mhCpuRtv[faceIndex][MipLevel];
}

D3D12_VIEWPORT FCubeRenderTarget::Viewport()const
{
	return mViewport;
}

D3D12_RECT FCubeRenderTarget::ScissorRect()const
{
	return mScissorRect;
}

void FCubeRenderTarget::BuildDescriptors(
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
	std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> hCpuRtv[6]
)
{
	// Save references to the descriptors. 
	mhCpuSrv = hCpuSrv;
	mhGpuSrv = hGpuSrv;


	for (int i = 0; i < 6; ++i)
	{
		assert(hCpuRtv[i].size() == mMipLevels);
		mhCpuRtv[i].resize(mMipLevels);
		for (UINT j = 0; j < mMipLevels; ++j)
		{
			mhCpuRtv[i][j] = hCpuRtv[i][j];
		}
	}

	//  Create the descriptors
	BuildDescriptors();
}

void FCubeRenderTarget::OnResize(UINT newWidth, UINT newHeight)
{
	if ((mWidth != newWidth) || (mHeight != newHeight))
	{
		mWidth = newWidth;
		mHeight = newHeight;

		BuildResource();

		// New resource, so we need new descriptors to that resource.
		BuildDescriptors();
	}
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
	mDevice->CreateShaderResourceView(mCubeMap.Get(), &srvDesc, mhCpuSrv);

	// Create RTV to each cube face.
	for (int i = 0; i < 6; ++i)
	{
		for (UINT j = 0; j < mMipLevels; ++j)
		{
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Format = mFormat;
			rtvDesc.Texture2DArray.PlaneSlice = 0;

			// Render target to ith element, jth mipmap.
			rtvDesc.Texture2DArray.FirstArraySlice = i;
			rtvDesc.Texture2DArray.MipSlice = j;

			// Only view one element of the array.
			rtvDesc.Texture2DArray.ArraySize = 1;

			// Create RTV to ith cubemap face.
			mDevice->CreateRenderTargetView(mCubeMap.Get(), &rtvDesc, mhCpuRtv[i][j]);
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

	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.DepthOrArraySize = 6;
	texDesc.MipLevels = mMipLevels;
	texDesc.Format = mFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_HEAP_PROPERTIES DefaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	THROW_IF_FAILED(
		mDevice->CreateCommittedResource(
			&DefaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mCubeMap)
		)
	);
}