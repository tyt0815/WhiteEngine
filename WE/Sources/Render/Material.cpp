#include "Material.h"
#include <DirectXColors.h>
#include "Texture.h"
#include "DirectX/DXResourceManager.h"

extern const int FrameResourcesNum;

//FMaterialManager* gMaterialManager = GetMaterialManager();

FMaterialManager::FMaterialManager()
{
	BuildMaterials();
}

FMaterialManager::~FMaterialManager()
{

}

void FMaterialManager::BuildMaterials()
{
	mMaterials.resize(EMT_None);
	BuildMaterial(
		EMT_Default,
		ETT_Default,
		-1,
		XMFLOAT4(Colors::White),
		XMFLOAT3(0.02f, 0.02f, 0.02f),
		0.1f,
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_Brick0,
		ETT_Bricks3,
		-1,
		XMFLOAT4(Colors::White),
		XMFLOAT3(0.02f, 0.02f, 0.02f),
		0.1f,
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_Grass,
		ETT_Grass,
		-1,
		XMFLOAT4(Colors::White),
		XMFLOAT3(0.01f, 0.01f, 0.01f),
		0.9f,
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_Stone0,
		ETT_Stone,
		-1,
		XMFLOAT4(Colors::White),
		XMFLOAT3(0.05f, 0.05f, 0.05f),
		0.3f,
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_Tile0,
		ETT_Tile,
		-1,
		XMFLOAT4(Colors::White),
		XMFLOAT3(0.02f, 0.02f, 0.02f),
		0.2f,
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_Skull,
		ETT_White,
		-1,
		XMFLOAT4(Colors::DarkGray),
		XMFLOAT3(0.05f, 0.05f, 0.05f),
		0.3f,
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_WireFence,
		ETT_WireFence,
		-1,
		XMFLOAT4(Colors::White),
		XMFLOAT3(0.05f, 0.05f, 0.05f),
		0.2f,
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_Water,
		ETT_Water,
		-1,
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f),
		XMFLOAT3(0.1f, 0.1f, 0.1f),
		0.0f,
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_Foliage1,
		ETT_Foliage1,
		-1,
		XMFLOAT4(Colors::White),
		XMFLOAT3(0.02f, 0.02f, 0.02f),
		0.1f,
		FDXMath::Identity4x4()
	);
}

void FMaterialManager::BuildMaterial(
	EMaterialType Type,
	int DiffuseSrvHeapIndex,
	int NormalSrvHeapIndex,
	DirectX::XMFLOAT4 DiffuseAlbedo,
	DirectX::XMFLOAT3 FresnelR0,
	float Roughness,
	DirectX::XMFLOAT4X4 MatTransform
)
{
	std::unique_ptr<FMaterial> Material = std::make_unique<FMaterial>();
	Material->Type = Type;
	Material->MatCBIndex = Type;
	Material->DiffuseSrvHeapIndex = DiffuseSrvHeapIndex;
	Material->NormalSrvHeapIndex = NormalSrvHeapIndex;
	Material->DiffuseAlbedo = DiffuseAlbedo;
	Material->FresnelR0 = FresnelR0;
	Material->Roughness = Roughness;
	Material->DirtyFrameCount = FrameResourcesNum;
	mMaterials[Type] = std::move(Material);
}
