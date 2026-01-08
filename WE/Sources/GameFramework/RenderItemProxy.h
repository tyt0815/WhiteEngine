#pragma once

#include "DirectX/DXMath.h"
#include <vector>

class FMeshGeometry;
class FMaterial;


struct FStaticMeshProxy
{
	FMeshGeometry* MeshGeometry;
	FMaterial* Material;
	size_t MeshCBIndex = 0;
	size_t SubmeshCBIndex = 0;
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	UINT BaseVertexLocation = 0;
	bool bCastShadow = false;
};

struct FMeshCBProxy
{
	DirectX::XMFLOAT4X4 World;
};

struct FSubmeshCBProxy
{
	UINT MaterialIndex;
};

struct FDirectionalLightProxy
{
	DirectX::XMFLOAT3 Direction;
	DirectX::XMFLOAT3 Color;
	bool bCastShadow;
	class FDepthStencil* ShadowMap;
};

class FRenderItemProxy
{
public:
	void Cleanup();

	size_t AllocateMeshCbProxy();

	size_t AllocateSubmeshCbProxy();

	size_t AllocateStaticMeshCbProxy();

	size_t AllocateDirectionalLightCbProxy();

// private:
	std::vector<FMeshCBProxy> mMeshCBProxies;

	std::vector<FSubmeshCBProxy> mSubmeshCBProxies;

	std::vector<FStaticMeshProxy> mStaticMeshProxies;

	std::vector<FDirectionalLightProxy> mDirectionalLightProxies;

public:
	inline FMeshCBProxy* GetMeshCBProxy(size_t i)
	{
		return &mMeshCBProxies[i];
	}

	inline FSubmeshCBProxy* GetSubmeshCBProxy(size_t i)
	{
		return &mSubmeshCBProxies[i];
	}

	inline FStaticMeshProxy* GetStaticMeshProxy(size_t i)
	{
		return &mStaticMeshProxies[i];
	}

	inline FDirectionalLightProxy* GetDirectionalLightProxy(size_t i)
	{
		return &mDirectionalLightProxies[i];
	}

	friend class FRenderItem;
};