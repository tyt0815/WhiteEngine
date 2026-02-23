#include "TestWorld.h"
#include "Actor/PhysicsBox.h"
#include "Actor/PhysicsSphere.h"
#include "Actor/Enemy.h"
#include "Actor/Alliance.h"
#include "Actor/BoxHitReactor.h"
#include "Actor/StateMachineActor.h"
#include "Actor/DenseBoxHitReactorManager.h"
#include "Pawn/PlayerPawn.h"

void WTestWorld::BeginPlay()
{	
	SetPlayer(SpawnActor<APlayerPawn>()->GetWeakPtr<APawn>());

	Super::BeginPlay();



	FActorSpawnParameter Param;
	Param.Transform.Translation = XMFLOAT3(0, -2, 40);
	mDenseBoxHitReactorManager = GetWorld()->SpawnActor<ADenseBoxHitReactorManager>(Param);
	mDenseBoxHitReactorManager->SetSize(XMINT3(10, 10, 1));
	mDenseBoxHitReactorManager->Reset();
}

void WTestWorld::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	constexpr float SpawnDelay = 2.5f;
	static float a = SpawnDelay;
	a += DeltaSecond;
}