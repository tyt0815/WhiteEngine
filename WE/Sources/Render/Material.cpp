#include "Material.h"
#include "ShaderStructures.h"
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
		ESM_DefaultLit,
		EBM_Opaque,
		ETT_Default,
		ETT_White,
		ETT_White,
		ETT_White,
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_RustedIron2,
		ESM_DefaultLit,
		EBM_Opaque,
		ETT_RustedIron2_BaseColor,
		ETT_RustedIron2_Metallic,
		ETT_RustedIron2_Normal,
		ETT_RustedIron2_Roughness,
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_ScuffedGold,
		ESM_DefaultLit,
		EBM_Opaque,
		ETT_ScuffedGold_BaseColor,
		ETT_ScuffedGold_Metallic,
		ETT_ScuffedGold_Normal,
		ETT_ScuffedGold_Roughness,
		FDXMath::Identity4x4()
	);
	BuildMaterial(
		EMT_IceField,
		ESM_DefaultLit,
		EBM_Opaque,
		ETT_IceField_BaseColor,
		ETT_IceField_Metallic,
		ETT_IceField_Normal,
		ETT_IceField_Roughness,
		FDXMath::Identity4x4()
	);
}

void FMaterialManager::BuildMaterial(
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
	Material->DirtyFrameCount = FrameResourcesNum;
	mMaterials[Type] = std::move(Material);
}
