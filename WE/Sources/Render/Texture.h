#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <wrl.h>
#include "DirectX/d3dx12.h"
#include "Utility/Class.h"

enum ETextureType : UINT16
{
	ETT_Default,
	ETT_White,
	ETT_RustedIron2_BaseColor,
	ETT_RustedIron2_Metallic,
	ETT_RustedIron2_Normal,
	ETT_RustedIron2_Roughness,
	ETT_ScuffedGold_BaseColor,
	ETT_ScuffedGold_Metallic,
	ETT_ScuffedGold_Normal,
	ETT_ScuffedGold_Roughness,
	ETT_IceField_BaseColor,
	ETT_IceField_Metallic,
	ETT_IceField_Normal,
	ETT_IceField_Roughness,
	ETT_None
};

enum ECubeTextureType : UINT16
{
	ECTT_Snow,
	ECTT_None
};

class FTexture
{
public:
	static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

	// Unique material name for lookup.
	//std::string Name;
	std::uint16_t SRVHeapIndex;
	std::wstring Filename;
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

class FTextureManager
{
	SINGLETON(FTextureManager);
public:

private:
	void BuildTextures(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	std::unique_ptr<FTexture> LoadTexture(
		std::uint16_t SRVHeapIndex,
		std::wstring Filename,
		ID3D12Device* Device,
		ID3D12GraphicsCommandList* CommandList
	);
	void BuildTexture(
		ETextureType Type,
		std::wstring Filename,
		ID3D12Device* Device, 
		ID3D12GraphicsCommandList* CommandList
	);
	void BuildCubeTexture(
		ECubeTextureType Type,
		std::wstring Filename,
		ID3D12Device* Device,
		ID3D12GraphicsCommandList* CommandList
	);
	void BuildShaderResourceDescriptorHeap();
	void BuildShaderResource();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSRVHeap;
	std::vector<std::unique_ptr<FTexture>> mTextures;
	std::vector<std::unique_ptr<FTexture>> mCubeTextures;
public:
	inline ID3D12DescriptorHeap* GetSRVHeapPtr() const
	{
		return mSRVHeap.Get();
	}
	inline FTexture* GetTexture(ETextureType Type) const
	{
		return mTextures[Type].get();
	}
	inline std::uint16_t GetTextureSRVHeapIndex(ETextureType Type) const
	{
		return mTextures[Type]->SRVHeapIndex;
	}
	inline FTexture* GetCubeTexture(ECubeTextureType Type) const
	{
		return mCubeTextures[Type].get();
	}
};

inline FTextureManager* GetTextureManager()
{
	return FTextureManager::GetInstance();
}