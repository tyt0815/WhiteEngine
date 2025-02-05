#include "Grid.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"

AGrid::AGrid()
{
	FMeshGeometry::MeshGeometryMap& Geometries = FMeshGeometry::MeshGeometries;
	Material = FMaterial::Materials[EMT_Grass].get();
	Geometry = Geometries["Shape"].get();
	TextureTransform.Scale = { 1.0f, 1.0f, 1.0f };
	IndexCount = Geometry->DrawArgs["Grid"].IndexCount;
	StartIndexLocation = Geometry->DrawArgs["Grid"].StartIndexLocation;
	BaseVertexLocation = Geometry->DrawArgs["Grid"].BaseVertexLocation;
}
