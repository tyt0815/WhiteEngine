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

class FTexture
{
public:
	static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

	// Unique material name for lookup.
	//std::string Name;
	ETextureType Type;
	std::wstring Filename;
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

class FTextureManager
{
	SINGLETON(FTextureManager);
public:

private:
	void LoadTextures(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	void LoadTexture(
		ETextureType Type,
		std::wstring Filename,
		ID3D12Device* Device, 
		ID3D12GraphicsCommandList* CommandList
	);
	void BuildShaderResourceDescriptorHeap();
	void BuildShaderResource();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSRVHeap;
	std::vector<std::unique_ptr<FTexture>> mTextures;
public:
	inline ID3D12DescriptorHeap* GetSRVHeapPtr() const
	{
		return mSRVHeap.Get();
	}
};

inline FTextureManager* GetTextureManager()
{
	return FTextureManager::GetInstance();
}