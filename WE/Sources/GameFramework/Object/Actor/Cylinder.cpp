#include "Cylinder.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"

ACylinder::ACylinder()
{
	FMeshGeometry::MeshGeometryMap& Geometries = FMeshGeometry::MeshGeometries;
	Material = GetMaterialManager()->GetMaterial(EMT_Brick0);
	Geometry = Geometries["Shape"].get();
	TextureTransform.Scale = { 1.0f, 1.0f, 1.0f };
	IndexCount = Geometry->DrawArgs["Cylinder"].IndexCount;
	StartIndexLocation = Geometry->DrawArgs["Cylinder"].StartIndexLocation;
	BaseVertexLocation = Geometry->DrawArgs["Cylinder"].BaseVertexLocation;
}