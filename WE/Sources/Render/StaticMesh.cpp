#include "StaticMesh.h"
#include "Material.h"
#include "MeshGeometry.h"

FStaticMeshManager::FStaticMeshManager()
{
	BuildStaticMeshs();
}

FStaticMeshManager::~FStaticMeshManager()
{
}

void FStaticMeshManager::BuildStaticMeshs()
{
	FStaticMesh StaticMesh;
	BuildStaticMesh("SM_RustedIron2Sphere", "Sphere", EMT_RustedIron2);
	BuildStaticMesh("SM_ScuffedGoldSphere", "Sphere", EMT_ScuffedGold);
	BuildStaticMesh("SM_IceFieldGrid", "Grid", EMT_IceField);
	BuildStaticMesh("SM_ThickMortarStonework", "Sphere", EMT_ThickMortarStonework);
	BuildStaticMesh("SM_ScuffedGoldBox", "Box", EMT_ScuffedGold);
	BuildStaticMesh("SM_DefaultFloor", "Floor", EMT_LaminateFlooringBrown);
	BuildStaticMesh("SM_MetalCylinder", "Cylinder", EMT_Black);
	BuildStaticMesh("SM_MetalRing", "Ring", EMT_ScuffedGold);
	BuildStaticMesh("SM_LaminateFlooringBrownBox", "Box", EMT_LaminateFlooringBrown);
	BuildStaticMesh("SM_BlackBox", "Box", EMT_Black);
	BuildStaticMesh("SM_WhiteBox", "Box", EMT_White);
	BuildStaticMesh("SM_RedBox", "Box", EMT_Red);
	BuildStaticMesh("SM_GreenBox", "Box", EMT_Green);
}

void FStaticMeshManager::BuildStaticMesh(const std::string& Name, std::string MeshName, EMaterialType MaterialType)
{
	FStaticMesh StaticMesh;
	StaticMesh.Geometry = GetMeshGeometryManager()->GetMeshGeometry(MeshName);
	StaticMesh.Material = GetMaterialManager()->GetMaterial(MaterialType);
	mStaticMeshs[Name] = StaticMesh;
}
