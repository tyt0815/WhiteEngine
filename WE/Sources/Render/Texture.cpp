#include "Texture.h"

#include "DDSTextureLoader.h"
#include "DirectX/DXException.h"
#include "DirectX/DXUtility.h"

FTexture::TextureMap FTexture::Textures = FTexture::TextureMap();

void FTexture::LoadTexture(ID3D12Device* Device, ID3D12GraphicsCommandList* CommandList)
{
	auto White = std::make_unique<FTexture>();
	White->Name = "White";
	White->Filename = L"./Textures/White.dds";
	THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(Device,
		CommandList, White->Filename.c_str(),
		White->Resource, White->UploadHeap));

	Textures[ETT_White] = std::move(White);

	auto woodCrateTex = std::make_unique<FTexture>();
	woodCrateTex->Name = "woodCrateTex";
	woodCrateTex->Filename = L"./Textures/WoodCrate01.dds";
	THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(Device,
		CommandList, woodCrateTex->Filename.c_str(),
		woodCrateTex->Resource, woodCrateTex->UploadHeap));

	Textures[ETT_WoodCrate] = std::move(woodCrateTex);

	auto Brick3Tex = std::make_unique<FTexture>();
	Brick3Tex->Name = "Brick3Tex";
	Brick3Tex->Filename = L"./Textures/bricks3.dds";
	THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(Device,
		CommandList, Brick3Tex->Filename.c_str(),
		Brick3Tex->Resource, Brick3Tex->UploadHeap));

	Textures[ETT_Bricks3] = std::move(Brick3Tex);

	auto Stone = std::make_unique<FTexture>();
	Stone->Name = "Stone";
	Stone->Filename = L"./Textures/stone.dds";
	THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(Device,
		CommandList, Stone->Filename.c_str(),
		Stone->Resource, Stone->UploadHeap));

	Textures[ETT_Stone] = std::move(Stone);

	auto Tile = std::make_unique<FTexture>();
	Tile->Name = "Tile";
	Tile->Filename = L"./Textures/tile.dds";
	THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(Device,
		CommandList, Tile->Filename.c_str(),
		Tile->Resource, Tile->UploadHeap));

	Textures[ETT_Tile] = std::move(Tile);

	auto Grass = std::make_unique<FTexture>();
	Grass->Name = "Grass";
	Grass->Filename = L"./Textures/grass.dds";
	THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(Device,
		CommandList, Grass->Filename.c_str(),
		Grass->Resource, Grass->UploadHeap));

	Textures[ETT_Grass] = std::move(Grass);

	auto WireFence = std::make_unique<FTexture>();
	WireFence->Name = "WireFence";
	WireFence->Filename = L"./Textures/WireFence.dds";
	THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(Device,
		CommandList, WireFence->Filename.c_str(),
		WireFence->Resource, WireFence->UploadHeap));

	Textures[ETT_WireFence] = std::move(WireFence);

	auto WaterTex = std::make_unique<FTexture>();
	WaterTex->Name = "WaterTex";
	WaterTex->Filename = L"./Textures/water1.dds";
	THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(Device,
		CommandList, WaterTex->Filename.c_str(),
		WaterTex->Resource, WaterTex->UploadHeap));

	Textures[ETT_Water] = std::move(WaterTex);

	auto Foliage1 = std::make_unique<FTexture>();
	Foliage1->Name = "Foliage1";
	Foliage1->Filename = L"./Textures/Foliage1.dds";
	THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(Device,
		CommandList, Foliage1->Filename.c_str(),
		Foliage1->Resource, Foliage1->UploadHeap));
	Textures[ETT_Foliage1] = std::move(Foliage1);

	auto Foliage2 = std::make_unique<FTexture>();
	Foliage2->Name = "Foliage2";
	Foliage2->Filename = L"./Textures/Foliage2.dds";
	THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(Device,
		CommandList, Foliage2->Filename.c_str(),
		Foliage2->Resource, Foliage2->UploadHeap));
	Textures[ETT_Foliage2] = std::move(Foliage2);

	auto DefualtTex = std::make_unique<FTexture>();
	DefualtTex->Name = "Default";
	DefualtTex->Filename = L"./Textures/checkboard.dds";
	THROW_IF_FAILED(DirectX::CreateDDSTextureFromFile12(Device,
		CommandList, DefualtTex->Filename.c_str(),
		DefualtTex->Resource, DefualtTex->UploadHeap));
	Textures[ETT_Default] = std::move(DefualtTex);
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> FTexture::GetStaticSamplers()
{
	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp 
	};
}
