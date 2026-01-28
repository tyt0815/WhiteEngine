#include "HitReactor.h"
#include "World/World.h"

void AHitReactor::OnHit(AActor* Instigator)
{
	GetWorld()->DrawDebugLine(GetActorLocation(), Instigator->GetActorLocation(), XMFLOAT4(0, 0, 1, 1), 2);
	// Destroy();
}
