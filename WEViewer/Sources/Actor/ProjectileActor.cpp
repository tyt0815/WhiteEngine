#include "ProjectileActor.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "GameFramework/Object/Component/ProjectileMovementComponent.h"

AProjectileActor::AProjectileActor()
{
	ProjComp = CreateComponent<WProjectileMovementComponent>();
	ProjComp->mVelocity = { 0.0f, 0.0f, 1.0f };
}

void AProjectileActor::SetLifeSpan(float LifeSpan)
{
	ProjComp->SetLifeSpan(LifeSpan);
}
