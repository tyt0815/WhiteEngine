#include "ProjectileSphere.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "GameFramework/Object/Component/ProjectileMovementComponent.h"

AProjectileSphere::AProjectileSphere()
{
	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_RustedIron2Sphere));

	WProjectileMovementComponent* ProjComp = CreateComponent<WProjectileMovementComponent>();
	ProjComp->mVelocity = { 0.0f, 0.0f, 1.0f };
}