#pragma once
#include <array>
#include <d3d12.h>
#include "Material.h"
#include "DirectX/DXMath.h"
#include "Utility/Class.h"
#include "Utility/Pool.h"

class FMeshGeometry;

struct FRenderItemInfo
{
	FMeshGeometry* MeshGeometry;
	FMaterial* Material;
	std::uint64_t MeshCBIndex;
	std::uint64_t SubmeshCBIndex;
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};

class FRenderItemManager
{
	SINGLETON(FRenderItemManager);
public:

private:
	std::array<std::array<TPool<FRenderItemInfo>, EBM_None>, ESM_None> mRenderItems;

public:
	inline size_t RegisterRenderItem(EShadingModel ShadingModel, EBlendMode BlendMode, const FRenderItemInfo& Info)
	{
		return mRenderItems[ShadingModel][BlendMode].Register(Info);
	}
	inline void RemoveRenderItem(EShadingModel ShadingModel, EBlendMode BlendMode, size_t i)
	{
		mRenderItems[ShadingModel][BlendMode].Remove(i);
	}
	inline const TPool<FRenderItemInfo>& GetRenderItems(EShadingModel ShadingModel, EBlendMode BlendMode) const
	{
		return mRenderItems[ShadingModel][BlendMode];
	}
};

inline FRenderItemManager* GetRenderItemManager()
{
	return FRenderItemManager::GetInstance();
}