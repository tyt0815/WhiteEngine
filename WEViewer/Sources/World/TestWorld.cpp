#include "TestWorld.h"
#include "Actor/BoxHitReactor.h"
#include "Actor/StateMachineActor.h"
#include "Actor/DenseBoxHitReactorManager.h"
#include "Actor/Button.h"
#include "Actor/Platform.h"
#include "Pawn/PlayerPawn.h"
#include "Character/PlayerCharacter.h"

void WTestWorld::BeginPlay()
{	
	SetPlayer(SpawnActor<APlayerCharacter>()->GetWeakPtr<APlayerCharacter>());

	Super::BeginPlay();

	FActorSpawnParameter Param;
	Param.Transform.Scale = XMFLOAT3(1, 2, 1);
	Param.Transform.Translation = XMFLOAT3(-40, 1, 40);
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);
	Param.Transform.Translation.x += 2;
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);

	Param.Transform.Translation.x = -10;
	Param.Transform.Translation.y = 2.5;
	Param.Transform.Translation.z = 20;
	Param.Transform.Scale = XMFLOAT3(0.5, 0.5, 0.5);
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);
	Param.Transform.Translation.y += 0.5;
	Param.Transform.Translation.z = 30;
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);

	Param.Transform.Translation = XMFLOAT3(0, -2, 40);
	mDenseBoxHitReactorManager = GetWorld()->SpawnActor<ADenseBoxHitReactorManager>(Param);
	mDenseBoxHitReactorManager->SetSize(XMINT3(10, 10, 1));
	mDenseBoxHitReactorManager->Reset();
	
	Param.Transform.Translation = XMFLOAT3(0, -2, 20);
	AButton* ResetButton = GetWorld()->SpawnActor<AButton>(Param);
	ResetButton->mOnButtonInteracted.AddLambda([this]() { mDenseBoxHitReactorManager->Reset(); });

	Param.Transform.Translation.y += 1;
	SpawnActor<APlatform>(Param);
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

		Param.Transform.Translation.x += 20;
		SpawnActorByFactory<AActor>("BP_LaunchedMissile", Param);

		Param.Transform.Translation.x += 8;
		SpawnActorByFactory<AActor>("BP_EearthquakeProjSpawner", Param);

		Param.Transform.Translation.x = -10;
		SpawnActorByFactory<AActor>("BP_RingProj", Param);

		Param.Transform.Translation.x = +10;
		SpawnActorByFactory<AActor>("BP_FuzeUlt", Param);

		a = 0;
	}
}