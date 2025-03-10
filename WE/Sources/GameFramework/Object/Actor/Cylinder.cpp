#include "Cylinder.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"

ACylinder::ACylinder()
{
	Material = GetMaterialManager()->GetMaterial(EMT_Brick0);
	TextureTransform.Scale = { 1.0f, 1.0f, 1.0f };
	Geometry = GetMeshGeometryManager()->GetMeshGeometry(EMGT_Cylinder);
	IndexCount = Geometry->DrawArgs[0].IndexCount;
	StartIndexLocation = Geometry->DrawArgs[0].StartIndexLocation;
	BaseVertexLocation = Geometry->DrawArgs[0].BaseVertexLocation;
}