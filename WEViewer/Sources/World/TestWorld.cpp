#include "TestWorld.h"
#include "Actor/PhysicsBox.h"
#include "Actor/PhysicsSphere.h"
#include "Actor/Enemy.h"
#include "Actor/Alliance.h"
#include "Actor/StateMachineActor.h"

void WTestWorld::BeginPlay()
{	
	// SetPlayer(SpawnActor<APlayerPawn>());

	Super::BeginPlay();

	FActorSpawnParameter Param;
	Param.Transform.Scale = XMFLOAT3(1, 2, 1);
	Param.Transform.Translation = XMFLOAT3(-40, 1, 40);
	GetWorld()->SpawnActor<AEnemy>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<AEnemy>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<AAlliance>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<AEnemy>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<AAlliance>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<AEnemy>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<AAlliance>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<AEnemy>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<AAlliance>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<AEnemy>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<AAlliance>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<AEnemy>(Param);


	SpawnActorByFactory<AActor>("BP_StateMachinProj");
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
		SpawnActorByFactory<AActor>("BP_TurnAroundProj", Param);

		Param.Transform.Translation.x += 4;
		SpawnActorByFactory<AActor>("BP_Missile", Param);

		a = 0;

	}
}