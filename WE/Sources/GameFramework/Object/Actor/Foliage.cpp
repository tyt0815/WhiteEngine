#include "Foliage.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"

AFoliage::AFoliage()
{
	Material = GetMaterialManager()->GetMaterial(EMT_Foliage1);
	TextureTransform.Scale = { 1.0f, 1.0f, 1.0f };
	Geometry = GetMeshGeometryManager()->GetMeshGeometry(EMGT_BillboardPoint);
	IndexCount = Geometry->DrawArgs[0].IndexCount;
	StartIndexLocation = Geometry->DrawArgs[0].StartIndexLocation;
	BaseVertexLocation = Geometry->DrawArgs[0].BaseVertexLocation;
}

void AFoliage::Tick(float Delta)
{
	Super::Tick(Delta);

	DirtyFrameCount = FrameResourcesNum;
}
