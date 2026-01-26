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
		"RustedIron2_Albedo",
		"RustedIron2_Metallic",
		"RustedIron2_Normal",
		"RustedIron2_Roughness",
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_ScuffedGold,
		ESM_DefaultLit,
		EBM_Opaque,
		"ScuffedGold_Albedo",
		"ScuffedGold_Metallic",
		"ScuffedGold_Normal",
		"ScuffedGold_Roughness",
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_IceField,
		ESM_DefaultLit,
		EBM_Opaque,
		"IceField_Albedo",
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

	BuildMaterial(
		EMT_Metal049A,
		ESM_DefaultLit,
		EBM_Opaque,
		"Metal049A_Albedo",
		"Metal049A_Metalness",
		"Metal049A_Normal",
		"Metal049A_Roughness",
		FDXMath::Identity4x4()
	);

	BuildMaterial(
		EMT_White,
		ESM_DefaultLit,
		EBM_Opaque,
		"White",
		"Black",
		"White",
		"White",
		FDXMath::Identity4x4()
	);

	BuildMaterial(
		EMT_Black,
		ESM_DefaultLit,
		EBM_Opaque,
		"Black",
		"Black",
		"White",
		"White",
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
	TUniquePtr<FMaterial> Material = std::make_unique<FMaterial>();
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
		GetTextureManager()->GetTexture(AlbedoTexture)->GetSRVHeapIndex(),
		GetTextureManager()->GetTexture(MetallicTexture)->GetSRVHeapIndex(),
		GetTextureManager()->GetTexture(NormalTexture)->GetSRVHeapIndex(),
		GetTextureManager()->GetTexture(RoughnessTexture)->GetSRVHeapIndex(),
		MatTransform
	);
}
