#include "Material.h"
#include <DirectXColors.h>
#include "DirectX/DXResourceManager.h"

extern const int gFrameResourcesNum;

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
		ESM_DefaultLit,
		EBM_Opaque,
		"Default",
		"White",
		"White",
		"White",
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_RustedIron2,
		ESM_DefaultLit,
		EBM_Opaque,
		"RustedIron2_BaseColor",
		"RustedIron2_Metallic",
		"RustedIron2_Normal",
		"RustedIron2_Roughness",
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_ScuffedGold,
		ESM_DefaultLit,
		EBM_Opaque,
		"ScuffedGold_BaseColor",
		"ScuffedGold_Metallic",
		"ScuffedGold_Normal",
		"ScuffedGold_Roughness",
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_IceField,
		ESM_DefaultLit,
		EBM_Opaque,
		"IceField_BaseColor",
		"IceField_Metallic",
		"IceField_Normal",
		"IceField_Roughness",
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_ThickMortarStonework,
		ESM_DefaultLit,
		EBM_Opaque,
		"ThickMortarStonework_Albedo",
		"ThickMortarStonework_Metallic",
		"ThickMortarStonework_Normal",
		"ThickMortarStonework_Roughness",
		FDXMath::Identity4x4()
	);

	BuildMaterial(
		EMT_LaminateFlooringBrown,
		ESM_DefaultLit,
		EBM_Opaque,
		"LaminateFlooringBrown_Albedo",
		"LaminateFlooringBrown_Metallic",
		"LaminateFlooringBrown_Normal",
		"LaminateFlooringBrown_Roughness",
		FDXMath::Identity4x4()
	);
}

void FMaterialManager::Internal_BuildMaterial(
	EMaterialType Type,
	EShadingModel ShadingModel,
	EBlendMode BlendMode,
	UINT AlbedoSRVHeapIndex,
	UINT MetallicSRVHeapIndex,
	UINT NormalSRVHeapIndex,
	UINT RoughnessSRVHeapIndex,
	DirectX::XMFLOAT4X4 MatTransform
)
{
	std::unique_ptr<FMaterial> Material = std::make_unique<FMaterial>();
	Material->Type = Type;
	Material->ShadingModel = ShadingModel;
	Material->BlendMode = BlendMode;
	Material->AlbedoSRVHeapIndex = AlbedoSRVHeapIndex;
	Material->MetallicSRVHeapIndex = MetallicSRVHeapIndex;
	Material->NormalSRVHeapIndex = NormalSRVHeapIndex;
	Material->RoughnessSRVHeapIndex = RoughnessSRVHeapIndex;
	Material->MatTransform = MatTransform;
	Material->DirtyFrameCount = gFrameResourcesNum;
	mMaterials[Type] = std::move(Material);
}

void FMaterialManager::BuildMaterial(
	EMaterialType Type,
	EShadingModel ShadingModel,
	EBlendMode BlendMode,
	std::string AlbedoTexture,
	std::string MetallicTexture,
	std::string NormalTexture,
	std::string RoughnessTexture,
	DirectX::XMFLOAT4X4 MatTransform
)
{
	Internal_BuildMaterial(
		Type,
		ShadingModel,
		BlendMode,
		GetTextureManager()->GetTexture2D(AlbedoTexture)->SRVHeapIndex,
		GetTextureManager()->GetTexture2D(MetallicTexture)->SRVHeapIndex,
		GetTextureManager()->GetTexture2D(NormalTexture)->SRVHeapIndex,
		GetTextureManager()->GetTexture2D(RoughnessTexture)->SRVHeapIndex,
		MatTransform
	);
}
