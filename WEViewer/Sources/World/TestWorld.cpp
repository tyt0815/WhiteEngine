#include "TestWorld.h"
#include "Actor/PhysicsBox.h"
#include "Actor/PhysicsSphere.h"
#include "Actor/HitReactor.h"
#include "Actor/ProjectileBase.h"
//#include "Pawn/PlayerPawn.h"

void WTestWorld::BeginPlay()
{	
	// SetPlayer(SpawnActor<APlayerPawn>());

	Super::BeginPlay();

	FActorSpawnParameter Param;
	Param.Transform.Translation = XMFLOAT3(-10, 5, 50);
	GetWorld()->SpawnActorByFactory<AHitReactor>("BP_Enemy", Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActorByFactory<AHitReactor>("BP_Alliance", Param);
}

void WTestWorld::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	constexpr float SpawnDelay = 2.5f;
	static float a = SpawnDelay;
	a += DeltaSecond;

	if (a > SpawnDelay)
	{
		FActorSpawnParameter Param;
		Param.Transform.Translation = XMFLOAT3(-50, 0, 10);
		SpawnActorByFactory<AProjectileBase>("BP_Projectile1", Param);

		Param.Transform.Translation.x += 2;
		SpawnActorByFactory<AProjectileBase>("BP_Projectile2", Param);

		Param.Transform.Translation.x += 2;
		SpawnActorByFactory<AProjectileBase>("BP_Projectile3", Param);

		Param.Transform.Translation.x += 2;
		SpawnActorByFactory<AProjectileBase>("BP_Projectile4", Param);

		a = 0;

	}
}