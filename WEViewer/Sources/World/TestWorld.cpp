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
		if (auto Proj = SpawnActorByFactory<AProjectileBase>("BP_Projectile").lock())
		{
			XMFLOAT3 Loc = XMFLOAT3(0, -1, 10);
			Proj->SetActorLocation(Loc);
			if (auto Target = SpawnActor<AActor>().lock())
			{
				Loc.y += 10;
				Loc.z -= 10;
				Target->SetActorLocation(Loc);
			}
		}
		a = 0;
	}
}