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
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);
	Param.Transform.Translation.y += 0.5;
	Param.Transform.Translation.z = 30;
	GetWorld()->SpawnActor<ABoxHitReactor>(Param);

	Param.Transform.Translation = XMFLOAT3(0, -2, 40);
	Param.Transform.Scale = XMFLOAT3(1, 1, 1);
	mDenseBoxHitReactorManager = GetWorld()->SpawnActor<ADenseBoxHitReactorManager>(Param);
	mDenseBoxHitReactorManager->SetSize(XMINT3(10, 10, 1));
	mDenseBoxHitReactorManager->Reset();
	
	Param.Transform.Translation = XMFLOAT3(0, -2, 20);
	AButton* ResetButton = GetWorld()->SpawnActor<AButton>(Param);
	ResetButton->mOnButtonInteracted.AddLambda([this]() { mDenseBoxHitReactorManager->Reset(); });


	XMFLOAT3 StartPos = SpawnPlatform(
		{0, -1, -5},
		1,
		2,2
	);
	SpawnPlatform(StartPos, 1);
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

XMFLOAT3 WTestWorld::SpawnPlatform(const XMFLOAT3& StartPos, int TotalFloors, int SideCountX, int SideCountZ, float PlatformScale, float HorizontalSpacing, float VerticalSpacing, std::vector<int> DirectionX, std::vector<int> DirectionZ)
{
	// 시작 위치 설정 (사용자님이 주신 값)
	XMFLOAT3 CurrentPos = StartPos;

	// 방향 벡터 (우 -> 상 -> 좌 -> 하 순서로 순환)
	// 0: +X, 1: +Z, 2: -X, 3: -Z
	int CurrentDirIdx = 0;
	int DirIdxModular = (int)min(DirectionX.size(), DirectionZ.size());
	int PlatformCount = 0;

	FActorSpawnParameter Param;
	Param.Transform.Scale.x *= PlatformScale;
	Param.Transform.Scale.z *= PlatformScale;

	for (int Floor = 0; Floor < TotalFloors; ++Floor)
	{
		for (int Side = 0; Side < 4; ++Side) // 사각형의 4변
		{
			// X방향 변일 때는 SideCountX만큼, Z방향 변일 때는 SideCountZ만큼 반복
			int Steps = (Side % 2 == 0) ? SideCountX : SideCountZ;

			for (int i = 0; i < Steps; ++i)
			{
				
				Param.Transform.Translation = CurrentPos;
				// 플랫폼 스폰
				SpawnActor<APlatform>(Param);

				// 다음 위치 계산
				// 1. 수평 이동
				CurrentPos.x += DirectionX[CurrentDirIdx] * HorizontalSpacing;
				CurrentPos.z += DirectionZ[CurrentDirIdx] * HorizontalSpacing;

				// 2. 수직 이동 (한 칸 배치할 때마다 위로)
				CurrentPos.y += VerticalSpacing;

				PlatformCount++;
			}

			// 한 변을 다 채웠으면 방향 전환
			CurrentDirIdx = (CurrentDirIdx + 1) % DirIdxModular;
		}
	}

	return CurrentPos;
}
