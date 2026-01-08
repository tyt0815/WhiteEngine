#include "ProjectileSphere.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "GameFramework/Object/Component/ProjectileMovementComponent.h"

AProjectileSphere::AProjectileSphere()
{
	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_RustedIron2Sphere));

	WProjectileMovementComponent* ProjComp = CreateComponent<WProjectileMovementComponent>();
	ProjComp->mVelocity = { 0.0f, 0.0f, 0.0005f };
}

void AProjectileSphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedTime += DeltaTime;

	if (ElapsedTime > 5)
	{
		Destroy();
	}
}
