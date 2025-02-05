#include "WaterBall.h"
#include "Render/Material.h"

AWaterBall::AWaterBall():
	Super()
{
	Material = FMaterial::Materials[EMaterialType::EMT_Water].get();
}

void AWaterBall::Tick(float Delta)
{
	Super::Tick(Delta);

	TextureTransform.Translation.x += 0.5f * Delta;
	TextureTransform.Translation.y += 0.5f * Delta;
	if (TextureTransform.Translation.x > 1.0f) TextureTransform.Translation.x -= 1.0f;
	if (TextureTransform.Translation.y > 1.0f) TextureTransform.Translation.y -= 1.0f;
	NumFramesDirty = NumFrameResources;
}
