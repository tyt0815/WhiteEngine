#include "Cylinder.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"

ACylinder::ACylinder()
{
	FMeshGeometry::MeshGeometryMap& Geometries = FMeshGeometry::MeshGeometries;
	FMaterial::MaterialMap& Materials = FMaterial::Materials;
	Material = Materials["Stone0"].get();
	Geometry = Geometries["Shape"].get();
	TextureTransform.Scale = { 1.0f, 1.0f, 1.0f };
	IndexCount = Geometry->DrawArgs["Box"].IndexCount;
	StartIndexLocation = Geometry->DrawArgs["Box"].StartIndexLocation;
	BaseVertexLocation = Geometry->DrawArgs["Box"].BaseVertexLocation;
}