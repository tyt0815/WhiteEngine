#pragma once
#include <array>
#include <d3d12.h>
#include "Material.h"
#include "DirectX/DXMath.h"
#include "Utility/Class.h"
#include "Utility/Pool.h"

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

struct FLightInfo
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Rotation;
	DirectX::XMFLOAT3 Color;
};

struct FDirectionalLightInfo
{
	UINT LightInfoIndex;
};

class FRenderItemManager
{
	SINGLETON(FRenderItemManager);
public:
	TPool<FMeshInfo> mMeshInfoPool;
	TPool<FSubmeshInfo> mSubmeshInfoPool;
	std::array<std::array<TPool<FStaticMeshInfo>, EBM_None>, ESM_None> mStaticMeshInfoPool;
	TPool<FLightInfo> mLightInfoPool;
	TPool<FDirectionalLightInfo> mDirectionalLightInfoPool;
};

inline FRenderItemManager* GetRenderItemManager()
{
	return FRenderItemManager::GetInstance();
}