//***************************************************************************************
// FCubeRenderTarget.h by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#pragma once
#include <array>
#include <vector>
#include "DirectX/DXMath.h"
#include "DirectX/DXUtility.h"
#include "Utility/Class.h"

class FTexture;
class FTextureManager;
class FDXResourceManager;

enum class CubeMapFace : int
{
	PositiveX = 0,
	NegativeX = 1,
	PositiveY = 2,
	NegativeY = 3,
	PositiveZ = 4,
	NegativeZ = 5
};

class FCubeRenderTarget : FNoncopyable
{
public:
	static std::array<DirectX::XMFLOAT4X4, 6> GetCubeMapViews();
	FCubeRenderTarget(
		std::string Name,
		UINT Width,
		UINT Height,
		UINT MipLevels = 1,
		DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT
	);
	FCubeRenderTarget() = delete;
	~FCubeRenderTarget();
	void OnResize(UINT Width, UINT Height);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(int FaceIndex, int MipLevel) const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCubeMapCPUDescriptorHeap() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetCubeMapGPUDescriptorHeap() const;
	D3D12_VIEWPORT GetViewportMipLevel(int i) const;
	D3D12_RECT GetScissorRectMipLevel(int i) const;

	FTexture* mTexture;
private:
	void Initialize();
	void BuildResource();
	void BuildRTVAndDSV();
	void BuildDescriptors();

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDSVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> mCubeMapResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilResource;

	std::string mName;
	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
	DXGI_FORMAT mFormat;
	DXGI_FORMAT mDepthStencilFormat;
	UINT mWidth = 0;
	UINT mHeight = 0;
	UINT mMipLevels;


public:
	inline UINT GetMipLevels() const
	{
		return mMipLevels;
	}
	inline ID3D12Resource* GetCubeMapResource() const
	{
		return mCubeMapResource.Get();
	}
	inline ID3D12Resource* GetDepthStencilResource() const
	{
		return mDepthStencilResource.Get();
	}
	inline ID3D12DescriptorHeap* GetRTVHeap() const
	{
		return mRTVHeap.Get();
	}
	inline ID3D12DescriptorHeap* GetDSVHeap() const
	{
		return mDSVHeap.Get();
	}
	inline D3D12_VIEWPORT GetViewport() const
	{
		return mViewport;
	}
	inline D3D12_RECT GetScissorRect() const
	{
		return mScissorRect;
	}
	inline DXGI_FORMAT GetFormat() const
	{
		return mFormat;
	}
	inline DXGI_FORMAT GetDepthStencilFormat() const
	{
		return mDepthStencilFormat;
	}
};

