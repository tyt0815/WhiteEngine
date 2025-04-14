#pragma once

#include <d3d12.h>
#include <memory>
#include <unordered_map>
#include <wrl.h>
#include "Texture.h"
#include "DirectX/DXMath.h"
#include "Utility/Class.h"
#include "Utility/String.h"

enum EShadingModel : std::uint16_t
{
	ESM_DefaultLit = 0,
	ESM_None
};

enum EBlendMode : std::uint16_t
{
	EBM_Opaque = 0,
	//EBM_AlphaTest,
	//EBM_Transparency,
	EBM_None
};

enum EMaterialType : UINT16
{
	EMT_Default = 0,
	EMT_RustedIron2,
	EMT_ScuffedGold,
	EMT_IceField,
	EMT_ThickMortarStonework,
	EMT_LaminateFlooringBrown,
	EMT_None
};

class FMaterial
{
public:
	EMaterialType Type;
	EShadingModel ShadingModel;
	EBlendMode BlendMode;
	UINT AlbedoSRVHeapIndex = -1;
	UINT MetallicSRVHeapIndex = -1;
	UINT NormalSRVHeapIndex = -1;
	UINT RoughnessSRVHeapIndex = -1;
	DirectX::XMFLOAT4X4 MatTransform = FDXMath::Identity4x4();
	int DirtyFrameCount = -1;

	friend class FMaterialManager;
};

class FMaterialManager
{
	SINGLETON(FMaterialManager);
public:
	
private:
	void BuildMaterials();
	void Internal_BuildMaterial(
		EMaterialType Type,
		EShadingModel ShadingModel,
		EBlendMode BlendMode,
		UINT AlbedoSRVHeapIndex,
		UINT MetallicSRVHeapIndex,
		UINT NormalSRVHeapIndex,
		UINT RoughnessSRVHeapIndex,
		DirectX::XMFLOAT4X4 MatTransform
	);
	void BuildMaterial(
		EMaterialType Type,
		EShadingModel ShadingModel,
		EBlendMode BlendMode,
		std::string AlbedoTexture,
		std::string MetallicTexture,
		std::string NormalTexture,
		std::string RoughnessTexture,
		DirectX::XMFLOAT4X4 MatTransform
	);
	std::vector<std::unique_ptr<FMaterial>> mMaterials;

public:
	inline FMaterial* GetMaterial(EMaterialType MaterialType)
	{
		return mMaterials[MaterialType].get();
	}
	inline FMaterial* GetMaterial(std::uint16_t i)
	{
		return mMaterials[i].get();
	}
};

inline FMaterialManager* GetMaterialManager()
{
	return FMaterialManager::GetInstance();
}