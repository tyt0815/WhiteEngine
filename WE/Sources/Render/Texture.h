#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <wrl.h>
#include "DirectX/d3dx12.h"
#include "Utility/Class.h"
#include "Utility/String.h"

constexpr UINT TEXTURE2D_NUM = 1000;
constexpr UINT TEXTURECUBE_NUM = 24;

class FTexture
{
public:
	static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

	// Unique material name for lookup.
	std::string Name;
	UINT SRVHeapIndex;
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

class FTextureManager
{
	SINGLETON(FTextureManager);
public:
	void RegisterTexture2D(std::unique_ptr<FTexture> Texture2D);
	void RegisterTextureCube(std::unique_ptr<FTexture> TextureCube);
	void UpdateTexture2D(std::string Name);
	void UpdateTextureCube(std::string Name);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(int i) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(int i) const;

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
	inline FTexture* GetTexture2D(std::string Name) const
	{
		return (*mTexture2Ds.find(Name)).second.get();
	}
	inline std::uint16_t GetTexture2DSRVHeapIndex(std::string Name) const
	{
		return (*mTexture2Ds.find(Name)).second->SRVHeapIndex;
	}
	inline FTexture* GetTextureCube(std::string Name) const
	{
		return (*mTextureCubes.find(Name)).second.get();
	}
};

inline FTextureManager* GetTextureManager()
{
	return FTextureManager::GetInstance();
}