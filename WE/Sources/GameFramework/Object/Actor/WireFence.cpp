#include "WireFence.h"
#include "Render/Material.h"

AWireFence::AWireFence():
	Super()
{
	Material = GetMaterialManager()->GetMaterial(EMT_WireFence);
}