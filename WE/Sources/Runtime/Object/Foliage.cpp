#include "Foliage.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"

AFoliage::AFoliage()
{
	FMeshGeometry::MeshGeometryMap& Geometries = FMeshGeometry::MeshGeometries;
	Material = FMaterial::Materials[EMT_Foliage1].get();
	Geometry = Geometries["Points"].get();
	TextureTransform.Scale = { 1.0f, 1.0f, 1.0f };
	IndexCount = Geometry->DrawArgs["Points"].IndexCount;
	StartIndexLocation = Geometry->DrawArgs["Points"].StartIndexLocation;
	BaseVertexLocation = Geometry->DrawArgs["Points"].BaseVertexLocation;
}

void AFoliage::Tick(float Delta)
{
	Super::Tick(Delta);

	NumFramesDirty = NumFrameResources;
}
