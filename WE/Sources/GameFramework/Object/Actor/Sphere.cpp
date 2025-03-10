#include "Sphere.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"

ASphere::ASphere():
	Super()
{
	FMeshGeometry::MeshGeometryMap& Geometries = FMeshGeometry::MeshGeometries;
	Material = GetMaterialManager()->GetMaterial(EMT_Stone0);
	Geometry = Geometries["Shape"].get();
	TextureTransform.Scale = { 1.0f, 1.0f, 1.0f };
	IndexCount = Geometry->DrawArgs["Sphere"].IndexCount;
	StartIndexLocation = Geometry->DrawArgs["Sphere"].StartIndexLocation;
	BaseVertexLocation = Geometry->DrawArgs["Sphere"].BaseVertexLocation;
}
