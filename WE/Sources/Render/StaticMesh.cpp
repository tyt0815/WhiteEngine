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
	BuildStaticMesh(ESMT_RustedIron2Sphere, EMGT_Sphere, EMT_RustedIron2);
	BuildStaticMesh(ESMT_ScuffedGoldSphere, EMGT_Sphere, EMT_ScuffedGold);
	BuildStaticMesh(ESMT_IceFieldGrid, EMGT_Grid, EMT_IceField);
}

void FStaticMeshManager::BuildStaticMesh(EStaticMeshType Type, EMeshGeometryType MeshType, EMaterialType MaterialType)
{
	FStaticMesh StaticMesh;
	StaticMesh.Geometry = GetMeshGeometryManager()->GetMeshGeometry(MeshType);
	StaticMesh.Material = GetMaterialManager()->GetMaterial(MaterialType);
	mStaticMeshs[Type] = StaticMesh;
}
