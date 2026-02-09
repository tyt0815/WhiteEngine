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

	constexpr float SpawnDelay = 10;
	static float a = SpawnDelay;
	a += DeltaSecond;

	if (a > SpawnDelay)
	{
		FActorSpawnParameter Param;
		Param.Transform.Translation = XMFLOAT3(0, 3, 10);
		SpawnActorByFactory<AProjectileBase>("BP_Projectile", Param);

		a = 0;

	}
}