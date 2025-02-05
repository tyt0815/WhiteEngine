#include "WireFence.h"
#include "Render/Material.h"

AWireFence::AWireFence():
	Super()
{
	Material = FMaterial::Materials[EMaterialType::EMT_WireFence].get();
}