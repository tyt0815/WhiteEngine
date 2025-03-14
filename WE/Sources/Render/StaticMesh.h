#pragma once
#include <d3d12.h>
#include <vector>
#include "DirectX/DXMath.h"
#include "Utility/Class.h"

class FMeshGeometry;
class FMaterial;

enum EStaticMeshType : std::uint16_t
{
	ESMT_Box,
	ESMT_Cylinder,
	ESMT_Ground,
	ESMT_Skull,
	ESMT_Sphere,
	ESMT_WaterBall,
	ESMT_WireFence,
	ESMT_RustedIron2,
	ESMT_None
};

struct FStaticMesh
{
	FMeshGeometry* Geometry = nullptr;
	FMaterial* Material = nullptr;
};

class FStaticMeshManager
{
	SINGLETON(FStaticMeshManager);
public:

private:
	void BuildStaticMeshs();
	std::vector<FStaticMesh> mStaticMeshs;
public:
	inline FStaticMesh GetStaticMesh(size_t i)
	{
		return mStaticMeshs[i];
	}
	inline FStaticMesh GetStaticMesh(EStaticMeshType Type)
	{
		return mStaticMeshs[Type];
	}
};

inline FStaticMeshManager* GetStaticMeshManager()
{
	return FStaticMeshManager::GetInstance();
}