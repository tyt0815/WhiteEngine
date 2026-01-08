#include "ProjectileSpawner.h"
#include "DirectX/DXMath.h"
#include "GameFramework/Object/World/World.h"
#include "ProjectileSphere.h"
#include "ProjectileBox.h"
#include "Bullet.h"

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
		SpawnRandomProjectile();
		//SpawnBullet();
		mCoolDown = 0;
	}
}

void AProjectileSpawner::SpawnRandomProjectile()
{
	int ActorSelector = FDXMath::Rand(0, 1);
	AProjectileActor* Proj;
	if (ActorSelector == 0)
	{
		Proj = GetWorld()->SpawnActor<AProjectileSphere>();
	}
	else if (ActorSelector == 1)
	{
		Proj = GetWorld()->SpawnActor<AProjectileBox>();
	}
	else
	{
		Proj = GetWorld()->SpawnActor<ABullet>();
	}

	float RandScale = (FDXMath::RandF() + 0.5f) / 2;
	XMFLOAT3 Scale = { RandScale, RandScale  ,RandScale };
	Proj->SetActorScale(Scale);
	Proj->SetActorRotation(GetActorRotation());
	Proj->SetActorLocation(GetActorLocation());

	float RandLifeSpan = FDXMath::RandF() * 10;
	Proj->SetLifeSpan(RandLifeSpan);
}

void AProjectileSpawner::SpawnBullet()
{
	ABullet* Bullet = GetWorld()->SpawnActor<ABullet>();
	Bullet->SetActorLocation(GetActorLocation());
	Bullet->SetActorRotation(GetActorRotation());
}
