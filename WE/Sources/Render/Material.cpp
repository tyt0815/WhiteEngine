#include "Material.h"
#include <DirectXColors.h>
#include "Texture.h"

FMaterial::MaterialMap FMaterial::Materials = FMaterial::MaterialMap();

void FMaterial::BuildMaterial()
{
	auto bricks0 = std::make_unique<FMaterial>();
	bricks0->Name = "Bricks0";
	bricks0->MatCBIndex = EMT_Brick0;
	bricks0->DiffuseSrvHeapIndex = ETT_Bricks3;
	bricks0->DiffuseAlbedo = XMFLOAT4(Colors::White);
	bricks0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	bricks0->Roughness = 0.1f;
	Materials[EMT_Brick0] = move(bricks0);

	auto Grass = std::make_unique<FMaterial>();
	Grass->Name = "Grass";
	Grass->MatCBIndex = EMT_Grass;
	Grass->DiffuseSrvHeapIndex = ETT_Grass;
	Grass->DiffuseAlbedo = XMFLOAT4(Colors::White);
	Grass->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	Grass->Roughness = 0.9f;
	Materials[EMT_Grass] = move(Grass);

	auto stone0 = std::make_unique<FMaterial>();
	stone0->Name = "Stone0";
	stone0->MatCBIndex = EMT_Stone0;
	stone0->DiffuseSrvHeapIndex = ETT_Stone;
	stone0->DiffuseAlbedo = XMFLOAT4(Colors::White);
	stone0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	stone0->Roughness = 0.3f;
	Materials[EMT_Stone0] = move(stone0);

	auto tile0 = std::make_unique<FMaterial>();
	tile0->Name = "Tile0";
	tile0->MatCBIndex = EMT_Tile0;
	tile0->DiffuseSrvHeapIndex = ETT_Tile;
	tile0->DiffuseAlbedo = XMFLOAT4(Colors::White);
	tile0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	tile0->Roughness = 0.2f;
	Materials[EMT_Tile0] = move(tile0);

	auto skullMat = std::make_unique<FMaterial>();
	skullMat->Name = "Skull";
	skullMat->MatCBIndex = EMT_Skull;
	skullMat->DiffuseSrvHeapIndex = ETT_White;
	skullMat->DiffuseAlbedo = XMFLOAT4(Colors::DarkGray);
	skullMat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	skullMat->Roughness = 0.3f;
	Materials[EMT_Skull] = move(skullMat);

	auto WireFence = std::make_unique<FMaterial>();
	WireFence->Name = "WireFence";
	WireFence->MatCBIndex = EMT_WireFence;
	WireFence->DiffuseSrvHeapIndex = ETT_WireFence;
	WireFence->DiffuseAlbedo = XMFLOAT4(Colors::White);
	WireFence->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	WireFence->Roughness = 0.2f;
	Materials[EMT_WireFence] = move(WireFence);

	auto Water = std::make_unique<FMaterial>();
	Water->Name = "Water";
	Water->MatCBIndex = EMT_Water;
	Water->DiffuseSrvHeapIndex = ETT_Water;
	Water->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	Water->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	Water->Roughness = 0.0f;
	Materials[EMT_Water] = move(Water);

	auto Foliage = std::make_unique<FMaterial>();
	Foliage->Name = "Foliage";
	Foliage->MatCBIndex = EMT_Foliage1;
	Foliage->DiffuseSrvHeapIndex = ETT_Foliage1;
	Foliage->DiffuseAlbedo = XMFLOAT4(Colors::White);
	Foliage->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	Foliage->Roughness = 0.1f;
	Materials[EMT_Foliage1] = move(Foliage);

	auto Default = std::make_unique<FMaterial>();
	Default->Name = "Default";
	Default->MatCBIndex = EMT_Default;
	Default->DiffuseSrvHeapIndex = ETT_Default;
	Default->DiffuseAlbedo = XMFLOAT4(Colors::White);
	Default->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	Default->Roughness = 0.1f;
	Materials[EMT_Default] = move(Default);
}
