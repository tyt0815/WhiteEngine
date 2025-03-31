#include "StaticMesh.h"
#include "Material.h"
#include "MeshGeometry.h"

FStaticMeshManager::FStaticMeshManager()
{
	mStaticMeshs.resize(ESMT_None);
	BuildStaticMeshs();
}

FStaticMeshManager::~FStaticMeshManager()
{
}

void FStaticMeshManager::BuildStaticMeshs()
{
	FStaticMesh StaticMesh;
	BuildStaticMesh(ESMT_RustedIron2Sphere, "Sphere", EMT_RustedIron2);
	BuildStaticMesh(ESMT_ScuffedGoldSphere, "Sphere", EMT_ScuffedGold);
	BuildStaticMesh(ESMT_IceFieldGrid, "Grid", EMT_IceField);
	BuildStaticMesh(ESMT_ThickMortarStonework, "Sphere", EMT_ThickMortarStonework);
	BuildStaticMesh(ESMT_ScuffedGoldBox, "Box", EMT_ScuffedGold);
}

void FStaticMeshManager::BuildStaticMesh(EStaticMeshType Type, std::string MeshName, EMaterialType MaterialType)
{
	FStaticMesh StaticMesh;
	StaticMesh.Geometry = GetMeshGeometryManager()->GetMeshGeometry(MeshName);
	StaticMesh.Material = GetMaterialManager()->GetMaterial(MaterialType);
	mStaticMeshs[Type] = StaticMesh;
}
