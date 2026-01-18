#pragma once

#include "DirectX/DXMath.h"
#include <vector>
#include "Utility/Container.h"

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

struct FDebugLine3DVBProxy
{
	XMFLOAT3 Start;
	XMFLOAT3 End;
	XMFLOAT4 Color;
	float LifeSpan;
};

class FRenderItemProxy
{
public:
	void Cleanup(float Delta);

	size_t AllocateMeshCbProxy();

	size_t AllocateSubmeshCbProxy();

	size_t AllocateStaticMeshCbProxy();

	size_t AllocateDirectionalLightCbProxy();

	std::vector<FMeshCBProxy> mMeshCBProxies;

	std::vector<FSubmeshCBProxy> mSubmeshCBProxies;

	std::vector<FStaticMeshProxy> mStaticMeshProxies;

	std::vector<FDirectionalLightProxy> mDirectionalLightProxies;

	TArray<FDebugLine3DVBProxy> mDebugLine3DProxies;

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