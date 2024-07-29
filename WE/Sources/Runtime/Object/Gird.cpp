#include "Gird.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"

AGrid::AGrid()
{
	FMeshGeometry::MeshGeometryMap& Geometries = FMeshGeometry::MeshGeometries;
	FMaterial::MaterialMap& Materials = FMaterial::Materials;
	Material = Materials["Tile0"].get();
	Geometry = Geometries["Shape"].get();
	TextureTransform.Scale = { 8.0f, 8.0f, 1.0f };
	IndexCount = Geometry->DrawArgs["Grid"].IndexCount;
	StartIndexLocation = Geometry->DrawArgs["Grid"].StartIndexLocation;
	BaseVertexLocation = Geometry->DrawArgs["Grid"].BaseVertexLocation;
}
