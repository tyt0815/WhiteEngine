#pragma once

#include <unordered_map>
#include <memory>
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
	EBM_AlphaTest,
	EBM_Transparency,
	EBM_None
};

enum EMaterialType : UINT16
{
	EMT_Default = 0,
	EMT_Tile0,
	EMT_Brick0,
	EMT_Stone0,
	EMT_Skull,
	EMT_Grass,
	EMT_WireFence,
	EMT_Water,
	EMT_Foliage1,
	EMT_None
};

class FMaterial
{
public:
	EMaterialType Type;
	int MatCBIndex = -1;
	int DiffuseSrvHeapIndex = -1;
	int NormalSrvHeapIndex = -1;
	int DirtyFrameCount = -1;
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = .25f;
	DirectX::XMFLOAT4X4 MatTransform = FDXMath::Identity4x4();

	friend class FMaterialManager;
};

class FMaterialManager
{
	SINGLETON(FMaterialManager);
public:
	
private:
	void BuildMaterials();
	void BuildMaterial(
		EMaterialType Type,
		int DiffuseSrvHeapIndex,
		int NormalSrvHeapIndex,
		DirectX::XMFLOAT4 DiffuseAlbedo,
		DirectX::XMFLOAT3 FresnelR0,
		float Roughness,
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