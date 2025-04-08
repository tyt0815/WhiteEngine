#pragma once

#include <vector>
#include <unordered_map>
#include "DirectX/DXUtility.h"
#include "Utility/Class.h"
#include "Utility/String.h"

class FTexture;

class FRenderTarget : FNoncopyable
{
	struct FResourceInfo
	{
		FTexture* Texture;
		D3D12_RESOURCE_STATES ResourceState;
		int Index;
	};
public:
	FRenderTarget(
		std::vector<std::string> Names,
		UINT Width,
		UINT Height,
		UINT MipLevels = 1,
		DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM
	);
	FRenderTarget() = delete;
	virtual ~FRenderTarget();
	void TransitResourceBarrier(ID3D12GraphicsCommandList* CommandList, std::string Name, D3D12_RESOURCE_STATES ResourceState);
	void TransitDepthStencilResourceBarrier(ID3D12GraphicsCommandList* CommandList, D3D12_RESOURCE_STATES ResourceState);
	void ClearRenderTarget(ID3D12GraphicsCommandList* CommandList, std::string Name, int MipLevel = 0);
	void ClearDepthStencil(ID3D12GraphicsCommandList* CommandList);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(std::string Name, int MipLevel);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHeap(std::string Name);
	D3D12_VIEWPORT GetViewportMipLevel(int i) const;
	D3D12_RECT GetScissorRectMipLevel(int i) const;
	

private:
	static float sClearColor[4];
	static int sRenderTargetCount;

	void Initialize(std::vector<std::string> Names);
	void BuildResource(std::vector<std::string> Names);
	void BuildRTHeapAndDSVHeap();
	void BuildDescriptors();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRTVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDSVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilResource;
	std::unordered_map<std::string, FResourceInfo> mResourceMap;
	std::string mDepthStencilTextureName;
	D3D12_RESOURCE_STATES mDepthStencilState;
	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
	DXGI_FORMAT mFormat;
	UINT mWidth = 0;
	UINT mHeight = 0;
	UINT mMipLevels;
public:
	inline UINT GetMipLevels() const
	{
		return mMipLevels;
	}
	inline FTexture* GetTexture(std::string Name)
	{
		return mResourceMap[Name].Texture;
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
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	}
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const
	{
		return mDSVHeap->GetCPUDescriptorHandleForHeapStart();
	}
};