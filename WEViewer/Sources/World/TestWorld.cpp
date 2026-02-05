#include "TestWorld.h"
#include "Actor/PhysicsBox.h"
#include "Actor/HitReactor.h"
#include "Actor/ProjectileBase.h"
#include "Pawn/PlayerPawn.h"

void WTestWorld::BeginPlay()
{	
	SetPlayer(SpawnActor<APlayerPawn>());

	Super::BeginPlay();

	
	FActorSpawnParameter Param;
	Param.Transform.Translation = XMFLOAT3(1, 1, 50);
	GetWorld()->SpawnActor<AHitReactor>(Param);
}

void WTestWorld::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	static float a = 32;
	a += DeltaSecond;

	if (a > 1)
	{
		FActorSpawnParameter Param;
		Param.Transform.Translation = XMFLOAT3(0, -1, 10);
		SpawnActorByFactory<AProjectileBase>("BP_Projectile", Param);

		Param.Transform.Translation = XMFLOAT3(5, -1, 10);
		SpawnActorByFactory<AProjectileBase>("BP_RingProjectile", Param);

		a = 0;
	}
}