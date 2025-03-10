#include "Skull.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"

ASkull::ASkull()
{
	Material = GetMaterialManager()->GetMaterial(EMT_Skull);
	TextureTransform = FTransform::Default;
	Geometry = GetMeshGeometryManager()->GetMeshGeometry(EMGT_Skull);
	IndexCount = Geometry->DrawArgs[0].IndexCount;
	StartIndexLocation = Geometry->DrawArgs[0].StartIndexLocation;
	BaseVertexLocation = Geometry->DrawArgs[0].BaseVertexLocation;
}
