#pragma once

#include "DirectX/d3dx12.h"
#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <wrl.h>

enum ETextureType : UINT16
{
	ETT_WoodCrate,
	ETT_Bricks3,
	ETT_Stone,
	ETT_Grass,
	ETT_Tile,
	ETT_White,
	ETT_WireFence,
	ETT_Water,
	ETT_Foliage1,
	ETT_Foliage2,
	ETT_Default
};

class FTexture
{
public:
	using TextureMap = std::unordered_map<ETextureType, std::unique_ptr<FTexture>>;
	static TextureMap Textures;
	static void LoadTexture(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList);
	static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

	// Unique material name for lookup.
	std::string Name;

	std::wstring Filename;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};