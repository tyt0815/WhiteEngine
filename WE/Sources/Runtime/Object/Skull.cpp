#include "Skull.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"

ASkull::ASkull()
{
	FMeshGeometry::MeshGeometryMap& Geometries = FMeshGeometry::MeshGeometries;
	FMaterial::MaterialMap& Materials = FMaterial::Materials;
	Material = Materials["Skull"].get();
	Geometry = Geometries["Skull"].get();
	TextureTransform.Scale = { 0.5f, 0.5f, 0.5f };
	TextureTransform.Translation = { 0.0f, 1.0f, 0.0f };
	IndexCount = Geometry->DrawArgs["Skull"].IndexCount;
	StartIndexLocation = Geometry->DrawArgs["Skull"].StartIndexLocation;
	BaseVertexLocation = Geometry->DrawArgs["Skull"].BaseVertexLocation;
}
