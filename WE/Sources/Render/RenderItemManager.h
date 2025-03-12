#pragma once
#include <d3d12.h>
#include "Material.h"
#include "DirectX/DXMath.h"
#include "Utility/Class.h"
#include "Utility/Pool.h"

class FMeshGeometry;

struct FPrimitiveDrawArguments
{
	FMeshGeometry* MeshGeometry;
	FMaterial* Material;
	std::uint32_t PrimitiveCBIndex;
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};

class FRenderItemManager
{
	SINGLETON(FRenderItemManager);
public:

private:
	TPool<FPrimitiveDrawArguments> mDrawArgsList;

public:
	inline size_t RegisterDrawArgs(const FPrimitiveDrawArguments& DrawArgs)
	{
		return mDrawArgsList.Register(DrawArgs);
	}
	inline void RemoveDrawArgs(size_t i)
	{
		mDrawArgsList.Remove(i);
	}
	inline const TPool<FPrimitiveDrawArguments>& GetDrawArgsList() const
	{
		return mDrawArgsList;
	}
	inline void SetDrawArgs(size_t i, const FPrimitiveDrawArguments& DrawArgs)
	{
		mDrawArgsList.SetItem(i, DrawArgs);
	}
};

inline FRenderItemManager* GetRenderItemManager()
{
	return FRenderItemManager::GetInstance();
}