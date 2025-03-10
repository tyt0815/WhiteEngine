#include "Sphere.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"

ASphere::ASphere():
	Super()
{
	Material = GetMaterialManager()->GetMaterial(EMT_Stone0);
	TextureTransform.Scale = { 1.0f, 1.0f, 1.0f };
	Geometry = GetMeshGeometryManager()->GetMeshGeometry(EMGT_Sphere);
	IndexCount = Geometry->DrawArgs[0].IndexCount;
	StartIndexLocation = Geometry->DrawArgs[0].StartIndexLocation;
	BaseVertexLocation = Geometry->DrawArgs[0].BaseVertexLocation;
}
