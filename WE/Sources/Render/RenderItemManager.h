#pragma once
#include <array>
#include <d3d12.h>
#include "Material.h"
#include "DirectX/DXMath.h"
#include "Utility/Class.h"
#include "Utility/Container.h"

class FMeshGeometry;

struct FStaticMeshInfo
{
	FMeshGeometry* MeshGeometry;
	FMaterial* Material;
	std::uint64_t MeshCBIndex;
	std::uint64_t SubmeshCBIndex;
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
	bool bCastShadow = false;
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

struct FDirectionalLightInfo
{
	DirectX::XMFLOAT3 Direction;
	UINT DirtyFrameCount;
	DirectX::XMFLOAT3 Color;
	bool bCastShadow;
	class FDepthStencil* ShadowMap;
};

class FRenderItemManager
{
	SINGLETON(FRenderItemManager);
public:
	TUnorderedArray<FMeshInfo> mMeshInfoPool;
	TUnorderedArray<FSubmeshInfo> mSubmeshInfoPool;
	std::array<std::array<TUnorderedArray<FStaticMeshInfo>, EBM_None>, ESM_None> mStaticMeshInfoPool;
	TUnorderedArray<FDirectionalLightInfo> mDirectionalLightInfoPool;
};

inline FRenderItemManager* GetRenderItemManager()
{
	return FRenderItemManager::GetInstance();
}