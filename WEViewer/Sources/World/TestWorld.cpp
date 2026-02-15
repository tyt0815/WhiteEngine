#include "TestWorld.h"
#include "Actor/PhysicsBox.h"
#include "Actor/PhysicsSphere.h"
#include "Actor/HitReactor.h"
#include "Actor/StateMachineActor.h"

void WTestWorld::BeginPlay()
{	
	// SetPlayer(SpawnActor<APlayerPawn>());

	Super::BeginPlay();

	//FActorSpawnParameter Param;
	//Param.Transform.Scale = XMFLOAT3(1, 2, 1);
	//Param.Transform.Translation = XMFLOAT3(-40, 1, 30);
	//GetWorld()->SpawnActorByFactory<AHitReactor>("BP_Enemy", Param);
	//Param.Transform.Translation.x += 2;
	//Param.Transform.Translation.z += 6;
	//GetWorld()->SpawnActorByFactory<AHitReactor>("BP_Enemy", Param);
	//Param.Transform.Translation.x += 6;
	//GetWorld()->SpawnActorByFactory<AHitReactor>("BP_Alliance", Param);
	//Param.Transform.Translation.x += 2;
	//GetWorld()->SpawnActorByFactory<AHitReactor>("BP_Enemy", Param);
	//Param.Transform.Translation.x += 2;
	//GetWorld()->SpawnActorByFactory<AHitReactor>("BP_Alliance", Param);
	//Param.Transform.Translation.x += 2;
	//GetWorld()->SpawnActorByFactory<AHitReactor>("BP_Enemy", Param);
}

void WTestWorld::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	constexpr float SpawnDelay = 2.5f;
	static float a = SpawnDelay;
	a += DeltaSecond;

	if (a > SpawnDelay)
	{
		//FActorSpawnParameter Param;
		//Param.Transform.Translation = XMFLOAT3(-50, 0, 10);
		//SpawnActorByFactory<AProjectileBase>("BP_Projectile1", Param);

		//Param.Transform.Translation.x += 2;
		//SpawnActorByFactory<AProjectileBase>("BP_Projectile2", Param);

		//Param.Transform.Translation.x += 2;
		//SpawnActorByFactory<AProjectileBase>("BP_Projectile3", Param);

		//Param.Transform.Translation.x += 10;
		//SpawnActorByFactory<AProjectileBase>("BP_Projectile4", Param);
		//SpawnActorByFactory<AProjectileBase>("BP_Projectile5", Param);

		//Param.Transform.Translation.x += 8;
		//SpawnActorByFactory<AProjectileBase>("BP_Projectile6", Param);

		//Param.Transform.Translation.x += 2;
		//SpawnActorByFactory<AProjectileBase>("BP_Projectile7", Param);

		//Param.Transform.Translation.x += 2;
		//SpawnActorByFactory<AProjectileBase>("BP_Projectile8", Param);
		//Param.Transform.Translation.x += 2;
		//SpawnActorByFactory<AProjectileBase>("BP_Projectile9", Param);

		//Param.Transform.Translation.x += 4;
		//SpawnActorByFactory<AProjectileBase>("BP_Projectile10", Param);

		//Param.Transform.Translation.x += 4;
		//SpawnActorByFactory<AProjectileBase>("BP_Projectile11", Param);

		//Param.Transform.Translation.x += 4;
		//SpawnActorByFactory<AProjectileBase>("BP_Projectile12", Param);

		SpawnActorByFactory<AActor>("BP_StateMachinProj");

		a = 0;

	}
}