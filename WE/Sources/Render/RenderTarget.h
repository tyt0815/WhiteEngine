#pragma once

#include <vector>
#include "DirectX/DXUtility.h"
#include "Utility/Class.h"
#include "Utility/String.h"

class FTexture;

class FRenderTarget : FNoncopyable
{
public:
	FRenderTarget(
		std::string Name,
		UINT Width,
		UINT Height,
		UINT MipLevels = 1,
		DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT
	);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(int MipLevel) const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHeap() const;
	D3D12_VIEWPORT GetViewportMipLevel(int i) const;
	D3D12_RECT GetScissorRectMipLevel(int i) const;
	

private:
	void Initialize();
	void BuildResource();
	void BuildRTVAndDSV();
	void BuildDescriptors();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDSVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilResource;
	FTexture* mTexture;
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
	inline ID3D12Resource* GetResource() const
	{
		return mResource.Get();
	}
	inline ID3D12Resource* GetDepthStencilResource() const
	{
		return mDepthStencilResource.Get();
	}
	inline D3D12_VIEWPORT GetViewport() const
	{
		return mViewport;
	}
	inline D3D12_RECT GetScissorRect() const
	{
		return mScissorRect;
	}
	inline ID3D12DescriptorHeap* GetDSVHeap() const
	{
		return mDSVHeap.Get();
	}
	inline DXGI_FORMAT GetFormat() const
	{
		return mFormat;
	}
	inline DXGI_FORMAT GetDepthStencilFormat() const
	{
		return mDepthStencilFormat;
	}
	inline FTexture* GetTexture() const
	{
		return mTexture;
	}
};