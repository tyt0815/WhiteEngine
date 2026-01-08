#include "ProjectileSpawner.h"
#include "DirectX/DXMath.h"
#include "GameFramework/Object/World/World.h"
#include "ProjectileSphere.h"

void AProjectileSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectileSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	mCoolDown += DeltaTime;
	if(mCoolDown > 1)
	{
		SpawnRandomScaledProjectile();
		mCoolDown = 0;
	}
}

void AProjectileSpawner::SpawnRandomScaledProjectile()
{
	AProjectileSphere* Proj = GetWorld()->SpawnActor<AProjectileSphere>();
	float RandScale = FDXMath::RandF() + 0.1f;
	XMFLOAT3 Scale = { RandScale, RandScale  ,RandScale };
	Proj->SetActorScale(Scale);
	Proj->SetActorRotation(GetActorRotation());
	Proj->SetActorLocation(GetActorLocation());
}
