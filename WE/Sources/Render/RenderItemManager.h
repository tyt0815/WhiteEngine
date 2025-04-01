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

struct FMeshInfo
{
	DirectX::XMFLOAT4X4 World;
	UINT DirtyFrameCount;
};

struct FSubmeshInfo
{
	UINT MaterialIndex;
	UINT SkyIrradianceCubeMapIndex;
	UINT SkySpecularCubeMapIndex;
	UINT DirtyFrameCount;
};

class FRenderItemManager
{
	SINGLETON(FRenderItemManager);
public:
	

private:
	std::array<std::array<TPool<FRenderItemInfo>, EBM_None>, ESM_None> mRenderItems;
	TPool<FMeshInfo> mMeshInfos;
	TPool<FSubmeshInfo> mSubmeshInfos;

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
	inline std::uint64_t RegisterMeshInfo()
	{
		return mMeshInfos.Register({});
	}
	inline void RemoveMeshInfo(std::uint64_t i)
	{
		mMeshInfos.Remove(i);
	}
	inline std::uint64_t GetMeshInfoPoolSize() const
	{
		return mMeshInfos.GetPoolSize();
	}
	inline FMeshInfo GetMeshInfo(std::uint64_t i) const
	{ 
		return mMeshInfos.GetItem(i);
	}
	inline void SetMeshInfo(std::uint64_t i, const FMeshInfo& MeshInfo)
	{
		mMeshInfos.SetItem(i, MeshInfo);
	}
	inline bool IsUsedMeshInfoPool(std::uint64_t i) const
	{
		return mMeshInfos.IsUsed(i);
	}
	inline std::uint64_t RegisterSubmeshInfo()
	{
		return mSubmeshInfos.Register({});
	}
	inline void RemoveSubmeshInfo(std::uint64_t i)
	{
		mSubmeshInfos.Remove(i);
	}
	inline std::uint64_t GetSubmeshInfoPoolSize() const
	{
		return mSubmeshInfos.GetPoolSize();
	}
	inline FSubmeshInfo GetSubmeshInfo(std::uint64_t i) const
	{
		return mSubmeshInfos.GetItem(i);
	}
	inline void SetSubmeshInfo(std::uint64_t i, const FSubmeshInfo& SubmeshInfo)
	{
		mSubmeshInfos.SetItem(i, SubmeshInfo);
	}
	inline bool IsUsedSubmeshInfoPool(std::uint64_t i) const
	{
		return mSubmeshInfos.IsUsed(i);
	}
};

inline FRenderItemManager* GetRenderItemManager()
{
	return FRenderItemManager::GetInstance();
}