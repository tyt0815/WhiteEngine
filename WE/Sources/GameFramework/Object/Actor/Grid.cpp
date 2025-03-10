#include "Grid.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"

AGrid::AGrid()
{
	Material = GetMaterialManager()->GetMaterial(EMT_Grass);
	TextureTransform.Scale = { 1.0f, 1.0f, 1.0f };
	Geometry = GetMeshGeometryManager()->GetMeshGeometry(EMGT_Grid);
	IndexCount = Geometry->DrawArgs[0].IndexCount;
	StartIndexLocation = Geometry->DrawArgs[0].StartIndexLocation;
	BaseVertexLocation = Geometry->DrawArgs[0].BaseVertexLocation;
}
