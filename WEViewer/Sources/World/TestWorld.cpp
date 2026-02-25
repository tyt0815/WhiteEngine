#include "TestWorld.h"
#include "Actor/BoxHitReactor.h"
#include "Actor/StateMachineActor.h"
#include "Actor/DenseBoxHitReactorManager.h"
#include "Actor/Button.h"
#include "Actor/Platform.h"
#include "Actor/ProjectileSpawner.h"
#include "Pawn/PlayerPawn.h"
#include "Character/PlayerCharacter.h"

XMVECTOR GetRightVector(FXMVECTOR vForward)
{
	XMVECTOR vNormalizedForward = XMVector3Normalize(vForward);
	XMVECTOR vUp = XMVectorSet(0, 1, 0, 0);

	// 외적으로 오른쪽 벡터 산출
	XMVECTOR vRight = XMVector3Cross(vUp, vNormalizedForward);

	// 수직 이동 시 예외 처리 (수평 성분이 없을 때)
	if (XMVector3LengthSq(vRight).m128_f32[0] < 0.0001f)
	{
		vRight = XMVectorSet(1, 0, 0, 0);
	}
	return XMVector3Normalize(vRight);
}

XMVECTOR GetLeftVector(FXMVECTOR vForward)
{
	return XMVectorNegate(GetRightVector(vForward));
}

XMVECTOR GetLocationUp(FXMVECTOR vBaseLoc, float Distance)
{
	return XMVectorMultiplyAdd(XMVectorSet(0, 1, 0, 0), XMVectorReplicate(Distance), vBaseLoc);
}

XMVECTOR GetLocationLeft(FXMVECTOR vBaseLoc, FXMVECTOR vForward, float Distance)
{
	XMVECTOR vRight = GetRightVector(vForward);
	return XMVectorMultiplyAdd(XMVectorNegate(vRight), XMVectorReplicate(Distance), vBaseLoc);
}

XMVECTOR GetLocationRight(FXMVECTOR vBaseLoc, FXMVECTOR vForward, float Distance)
{
	XMVECTOR vRight = GetRightVector(vForward);
	return XMVectorMultiplyAdd(vRight, XMVectorReplicate(Distance), vBaseLoc);
}

XMVECTOR GetEulerFromDir(FXMVECTOR vDir)
{
	XMVECTOR vNormalizedDir = XMVector3Normalize(vDir);
	XMFLOAT3 dir;
	XMStoreFloat3(&dir, vNormalizedDir);

	// Yaw: X-Z 평면상의 각도
	float yaw = XMConvertToDegrees(atan2f(dir.x, dir.z));

	// Pitch: 수평 대비 수직 각도
	float horizontalLen = sqrtf(dir.x * dir.x + dir.z * dir.z);
	float pitch = XMConvertToDegrees(atan2f(-dir.y, horizontalLen));

	return XMVectorSet(pitch, yaw, 0.0f, 0.0f);
}

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


	XMFLOAT3 PlatformPos = { 0, -2, -0 };
	
	// Stage1
	{
		SpawnPlatforms(PlatformPos, 5, { 0, 1, -3 });
		SpawnPlatforms(PlatformPos, 10, { 0, 0, -4 });

		const FStageInfo* Info = &mStageInfos.back();
		XMVECTOR vStageDir = XMLoadFloat3(&Info->Dir);
		for (int i = Info->StartIdx; i < Info->EndIdx - 1; i += 2)
		{
			APlatform* TargetPlatform = mPlatforms[i];
			XMFLOAT3 TargetLoc = TargetPlatform->GetActorLocation();
			XMVECTOR vTargetLoc = XMLoadFloat3(&TargetLoc);
			FActorSpawnParameter TrapParam;
			XMStoreFloat3(&TrapParam.Transform.Translation, GetLocationLeft(vTargetLoc, vStageDir, 10));
			TrapParam.Transform.Translation.y += 1.0f;
			XMStoreFloat3(&TrapParam.Transform.Rotation, GetEulerFromDir(GetRightVector(vStageDir)));
			AProjectileSpawner* Spawner = SpawnActor<AProjectileSpawner>(TrapParam);
			Spawner->SetupSpawner("BP_Trap_LinearProj", 0, 2, true);
		}
		for (int i = Info->StartIdx + 1; i < Info->EndIdx - 1; i += 2)
		{
			APlatform* TargetPlatform = mPlatforms[i];
			XMFLOAT3 TargetLoc = TargetPlatform->GetActorLocation();
			XMVECTOR vTargetLoc = XMLoadFloat3(&TargetLoc);
			FActorSpawnParameter TrapParam;
			XMStoreFloat3(&TrapParam.Transform.Translation, GetLocationRight(vTargetLoc, vStageDir, 10));
			TrapParam.Transform.Translation.y += 1.0f;
			XMStoreFloat3(&TrapParam.Transform.Rotation, GetEulerFromDir(GetLeftVector(vStageDir)));
			AProjectileSpawner* Spawner = SpawnActor<AProjectileSpawner>(TrapParam);
			Spawner->SetupSpawner("BP_Trap_LinearProj", 1, 2, true);
		}
	}

	// Stage3
	{
		SpawnPlatforms(PlatformPos, 5, { 3, 1, 0 });
		SpawnPlatforms(PlatformPos, 10, { 0, 0, 4 });
		const FStageInfo* Info = &mStageInfos.back();
		XMVECTOR vStageDir = XMLoadFloat3(&Info->Dir);
		float Dist = 3;
		float YOffset = 0.5f;
		for (int i = Info->StartIdx; i < Info->EndIdx - 1; i += 2)
		{
			APlatform* TargetPlatform = mPlatforms[i];
			XMFLOAT3 TargetLoc = TargetPlatform->GetActorLocation();
			XMVECTOR vTargetLoc = XMLoadFloat3(&TargetLoc);
			FActorSpawnParameter TrapParam;
			XMStoreFloat3(&TrapParam.Transform.Translation, GetLocationLeft(vTargetLoc, vStageDir, Dist));
			TrapParam.Transform.Translation.y += YOffset;
			XMStoreFloat3(&TrapParam.Transform.Rotation, GetEulerFromDir(GetRightVector(vStageDir)));
			AProjectileSpawner* Spawner = SpawnActor<AProjectileSpawner>(TrapParam);
			Spawner->SetupSpawner("BP_Trap_TurnAroundProj", 0, 2, false);
		}
		for (int i = Info->StartIdx + 1; i < Info->EndIdx - 1; i += 2)
		{
			APlatform* TargetPlatform = mPlatforms[i];
			XMFLOAT3 TargetLoc = TargetPlatform->GetActorLocation();
			XMVECTOR vTargetLoc = XMLoadFloat3(&TargetLoc);
			FActorSpawnParameter TrapParam;
			XMStoreFloat3(&TrapParam.Transform.Translation, GetLocationRight(vTargetLoc, vStageDir, Dist));
			TrapParam.Transform.Translation.y += YOffset;
			XMStoreFloat3(&TrapParam.Transform.Rotation, GetEulerFromDir(GetLeftVector(vStageDir)));
			AProjectileSpawner* Spawner = SpawnActor<AProjectileSpawner>(TrapParam);
			Spawner->SetupSpawner("BP_Trap_TurnAroundProj", 0, 2, false);
		}
	}

	// Stage5
	{
		SpawnPlatforms(PlatformPos, 5, { -3, 1, 0 });
		SpawnPlatforms(PlatformPos, 10, { 0, 0, -5 }, 5);

		const FStageInfo* Info = &mStageInfos.back();
		XMVECTOR vStageDir = XMLoadFloat3(&Info->Dir);
		APlatform* LastPlatform = mPlatforms[Info->EndIdx - 2];
		
		float YOffset = 1.5f;
		FActorSpawnParameter TrapParam;
		TrapParam.Transform.Translation = LastPlatform->GetActorLocation();
		TrapParam.Transform.Translation.x -= 2;
		TrapParam.Transform.Translation.y += YOffset;
		XMStoreFloat3(&TrapParam.Transform.Rotation, GetEulerFromDir(-vStageDir));

		AProjectileSpawner* Spawner = SpawnActor<AProjectileSpawner>(TrapParam);
		Spawner->SetupSpawner("BP_Trap_BounceProj", FDXMath::RandF(0, 1), 1, 2, true);

		TrapParam.Transform.Translation.x += 1;
		Spawner = SpawnActor<AProjectileSpawner>(TrapParam);
		Spawner->SetupSpawner("BP_Trap_BounceProj", FDXMath::RandF(0, 1), 1, 2, true);

		TrapParam.Transform.Translation.x += 1;
		Spawner = SpawnActor<AProjectileSpawner>(TrapParam);
		Spawner->SetupSpawner("BP_Trap_BounceProj", FDXMath::RandF(0, 1), 1, 2, true);

		TrapParam.Transform.Translation.x += 1;
		Spawner = SpawnActor<AProjectileSpawner>(TrapParam);
		Spawner->SetupSpawner("BP_Trap_BounceProj", FDXMath::RandF(0, 1), 1, 2, true);

		TrapParam.Transform.Translation.x += 1;
		Spawner = SpawnActor<AProjectileSpawner>(TrapParam);
		Spawner->SetupSpawner("BP_Trap_BounceProj", FDXMath::RandF(0, 1), 1, 2, true);
	}
}

void WTestWorld::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	static float ElapsedTime = 0;
	ElapsedTime += DeltaSecond;

	constexpr float SpawnDelay = 2.5f;
	static float SpawnTime = SpawnDelay;
	SpawnTime += DeltaSecond;

	if (SpawnTime > SpawnDelay)
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

		

		

		SpawnTime = 0;
	}


}

XMFLOAT3 WTestWorld::SpawnPlatform(const XMFLOAT3& StartPos, int TotalFloors, int SideCountX, int SideCountZ, float PlatformScale, float HorizontalSpacing, float VerticalSpacing, std::vector<int> DirectionX, std::vector<int> DirectionZ)
{
	XMFLOAT3 CurrentPos = StartPos;

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

void WTestWorld::SpawnPlatforms(XMFLOAT3& Loc, int Count, const XMFLOAT3& Offset, float Scale)
{
	FStageInfo Info;
	Info.StartIdx = (int)mPlatforms.size();
	Info.EndIdx = Info.StartIdx + Count;
	Info.Dir = Offset;

	FTransform Transform;
	Transform.Scale.x = Transform.Scale.z = Scale;
	
	const XMVECTOR vOffset = XMLoadFloat3(&Offset);
	for (int i = 0; i < Count; ++i)
	{
		XMVECTOR vLoc = XMLoadFloat3(&Loc);
		XMStoreFloat3(&Loc, vLoc + vOffset);
		Transform.Translation = Loc;
		SpawnPlatform(Transform);
	}

	mStageInfos.push_back(std::move(Info));
}

void WTestWorld::SpawnPlatform(const FTransform& Transform)
{
	FActorSpawnParameter Param;
	Param.Transform = Transform;
	mPlatforms.push_back(SpawnActor<APlatform>(Param));
}
