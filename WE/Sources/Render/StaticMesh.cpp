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
	StaticMesh.Geometry = GetMeshGeometryManager()->GetMeshGeometry(EMGT_Box);
	StaticMesh.Material = GetMaterialManager()->GetMaterial(EMT_Stone0);
	mStaticMeshs[ESMT_Box] = StaticMesh;
	StaticMesh.Geometry = GetMeshGeometryManager()->GetMeshGeometry(EMGT_Cylinder);
	StaticMesh.Material = GetMaterialManager()->GetMaterial(EMT_Brick0);
	mStaticMeshs[ESMT_Cylinder] = StaticMesh;
	StaticMesh.Geometry = GetMeshGeometryManager()->GetMeshGeometry(EMGT_Grid);
	StaticMesh.Material = GetMaterialManager()->GetMaterial(EMT_Grass);
	mStaticMeshs[ESMT_Ground] = StaticMesh;
	StaticMesh.Geometry = GetMeshGeometryManager()->GetMeshGeometry(EMGT_Skull);
	StaticMesh.Material = GetMaterialManager()->GetMaterial(EMT_Skull);
	mStaticMeshs[ESMT_Skull] = StaticMesh;
	StaticMesh.Geometry = GetMeshGeometryManager()->GetMeshGeometry(EMGT_Sphere);
	StaticMesh.Material = GetMaterialManager()->GetMaterial(EMT_Tile0);
	mStaticMeshs[ESMT_Sphere] = StaticMesh;
	StaticMesh.Geometry = GetMeshGeometryManager()->GetMeshGeometry(EMGT_Sphere);
	StaticMesh.Material = GetMaterialManager()->GetMaterial(EMT_Water);
	mStaticMeshs[ESMT_WaterBall] = StaticMesh;
	StaticMesh.Geometry = GetMeshGeometryManager()->GetMeshGeometry(EMGT_Box);
	StaticMesh.Material = GetMaterialManager()->GetMaterial(EMT_WireFence);
	mStaticMeshs[ESMT_WireFence] = StaticMesh;
	StaticMesh.Geometry = GetMeshGeometryManager()->GetMeshGeometry(EMGT_Sphere);
	StaticMesh.Material = GetMaterialManager()->GetMaterial(EMT_RustedIron2);
	mStaticMeshs[ESMT_RustedIron2] = StaticMesh;
}