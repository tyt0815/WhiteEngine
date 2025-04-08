#pragma once

#include <memory>
#include <unordered_map>
#include <wrl.h>
#include "DirectX/d3dx12.h"
#include "Utility/Class.h"
#include "Utility/String.h"

constexpr UINT TEXTURE2D_NUM = 1024;
constexpr UINT TEXTURECUBE_NUM = 1024;

class FTexture
{
public:

	// Unique material name for lookup.
	std::string Name;
	UINT SRVHeapIndex = -1;
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

class FTextureManager
{
	SINGLETON(FTextureManager);
public:
	void RegisterTexture2D(std::unique_ptr<FTexture> Texture2D);
	void RegisterTextureCube(std::unique_ptr<FTexture> TextureCube);
	void RegisterDepthStencilTexture2D(std::unique_ptr<FTexture> Texture);
	void UpdateTexture2D(std::string Name);
	void UpdateTextureCube(std::string Name);
	D3D12_CPU_DESCRIPTOR_HANDLE GetTexture2DCPUSRVForHeapStart() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureCubeCPUSRVForHeapStart() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetTexture2DGPUSRVForHeapStart() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureCubeGPUSRVForHeapStart() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetTexture2DCPUDescriptorHandle(int i) const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureCubeCPUDescriptorHandle(int i) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetTexture2DGPUDescriptorHandle(int i) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureCubeGPUDescriptorHandle(int i) const;
	D3D12_DESCRIPTOR_RANGE GetTexture2DDescriptorRange() const;
	D3D12_DESCRIPTOR_RANGE GetTextureCubeDescriptorRange() const;

private:
	void BuildTextures(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	std::unique_ptr<FTexture> LoadTexture(
		std::string Name,
		std::wstring Filename,
		ID3D12Device* Device,
		ID3D12GraphicsCommandList* CommandList
	);
	void BuildTexture(
		std::string Name,
		std::wstring Filename,
		ID3D12Device* Device, 
		ID3D12GraphicsCommandList* CommandList
	);
	void BuildCubeTexture(
		std::string Name,
		std::wstring Filename,
		ID3D12Device* Device,
		ID3D12GraphicsCommandList* CommandList
	);
	void BuildShaderResourceDescriptorHeap();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSRVHeap;
	std::unordered_map<std::string, std::unique_ptr<FTexture>> mTexture2Ds;
	std::unordered_map<std::string, std::unique_ptr<FTexture>> mTextureCubes;
public:
	inline ID3D12DescriptorHeap* GetSRVHeapPtr() const
	{
		return mSRVHeap.Get();
	}
	inline FTexture* GetTexture2D(std::string Name)
	{
		return mTexture2Ds[Name].get();
	}
	inline FTexture* GetTextureCube(std::string Name)
	{
		return mTextureCubes[Name].get();
	}
};

inline FTextureManager* GetTextureManager()
{
	return FTextureManager::GetInstance();
}