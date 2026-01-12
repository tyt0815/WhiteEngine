#pragma once
#include <d3d12.h>
#include <vector>
#include "DirectX/DXMath.h"
#include "Utility/Class.h"
#include "MeshGeometry.h"
#include "Material.h"

class FMeshGeometry;
class FMaterial;

enum EStaticMeshType : std::uint16_t
{
	ESMT_RustedIron2Sphere,
	ESMT_ScuffedGoldSphere,
	ESMT_IceFieldGrid,
	ESMT_ThickMortarStonework,
	ESMT_ScuffedGoldBox,
	ESMT_LaminateFloorBrown,
	ESMT_RustedIron2Cylinder,
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
	void BuildStaticMesh(EStaticMeshType Type, std::string MeshName, EMaterialType MaterialType);
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