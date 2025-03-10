#include "Skull.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"

ASkull::ASkull()
{
	FMeshGeometry::MeshGeometryMap& Geometries = FMeshGeometry::MeshGeometries;
	Material = GetMaterialManager()->GetMaterial(EMT_Skull);
	Geometry = Geometries["Skull"].get();
	TextureTransform = FTransform::Default;
	IndexCount = Geometry->DrawArgs["Skull"].IndexCount;
	StartIndexLocation = Geometry->DrawArgs["Skull"].StartIndexLocation;
	BaseVertexLocation = Geometry->DrawArgs["Skull"].BaseVertexLocation;
}
