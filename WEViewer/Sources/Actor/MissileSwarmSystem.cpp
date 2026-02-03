#include "MissileSwarmSystem.h"
#include "World/World.h"
#include "TopAttackMissile.h"

AMissileSwarmSystem::AMissileSwarmSystem()
{
	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_High);
}

void AMissileSwarmSystem::Tick(float Delta)
{
	Super::Tick(Delta);

	if (!mbIsLaunching)
	{
		return;
	}

	mElapsedTime += Delta;
	int TargetIndex = min((int)mTargetMarkers.size() - 1, (int)(mElapsedTime / mFireDelay));
	int Row = (int)mTargetMarkers.size();
	int Col = (int)mTargetMarkers[0].size();

	for (int r = mLastFiredRow + 1; r <= TargetIndex; ++r)
	{
		for (int c = 0; c < mTargetMarkers[r].size(); ++c)
		{
			if (auto Marker = mTargetMarkers[r][c].lock())
			{
				FActorSpawnParameter Param;
				Param.Transform = GetActorTransform();
				Param.Transform.Translation = GetGridLocationAtIndex(r, c, Col, 0.3f, GetActorLocation(), XMFLOAT3(0, 1, 0), GetRightVector());
				if (auto Missile = GetWorld()->SpawnActorByFactory<ATopAttackMissile>("BP_TopAttackMissile", Param).lock())
				{
					TArray<TWeakPtr<AActor>> HomingPaths;
					CreateHomingPaths(mTargetMarkers[r][c], HomingPaths);

					Missile->Initialize(HomingPaths, mMissileGridManager.lock().get());
				}
			}
		}
	}

	mLastFiredRow = TargetIndex;

	if (mLastFiredRow + 1 >= mTargetMarkers.size())
	{
		mbIsLaunching = false;
		mMissileGridManager.reset();
		mTargetMarkers.clear();
	}
		
}

void AMissileSwarmSystem::ClearTargetMarkers()
{
	for (auto Row : mTargetMarkers)
	{
		for (auto Weak : Row)
		{
			if (auto Marker = Weak.lock())
			{
				Marker->Destroy();
			}
		}
	}

	mTargetMarkers.clear();
}

bool AMissileSwarmSystem::TryCreateTargetMarkers(int Row, int Col)
{
	if (!mTargetMarkers.empty())
	{
		return false;
	}

	mTargetMarkers.resize(Row);
	for (int r = 0; r < Row; ++r)
	{
		mTargetMarkers[r].resize(Col);
		for (int c = 0; c < Col; ++c)
		{
			mTargetMarkers[r][c] = GetWorld()->SpawnActor<ATargetMarker>();
		}
	}

	return true;
}

void AMissileSwarmSystem::CalcGridLocation(int Row, int Col, float GridInterval, XMFLOAT3 Origin, XMFLOAT3 AxisForward, XMFLOAT3 AxisRight, std::vector<std::vector<XMFLOAT3>>& OutLocations)
{
	// 1. 크기 미리 할당 (std::vector의 resize 사용)
	OutLocations.clear();
	OutLocations.resize(Row);

	XMVECTOR OriginV = XMLoadFloat3(&Origin);
	XMVECTOR ForwardV = XMLoadFloat3(&AxisForward);
	XMVECTOR RightV = XMLoadFloat3(&AxisRight);

	for (int r = 0; r < Row; ++r)
	{
		OutLocations[r].resize(Col);
		float ForwardOffset = r * GridInterval;

		for (int c = 0; c < Col; ++c)
		{
			// 중앙 정렬 오프셋
			float RightOffset = (c - (Col - 1) * 0.5f) * GridInterval;

			XMVECTOR PosV = OriginV + (ForwardV * ForwardOffset) + (RightV * RightOffset);

			// 주소 연산 규칙 준수
			XMStoreFloat3(&OutLocations[r][c], PosV);
		}
	}
}

XMFLOAT3 AMissileSwarmSystem::GetGridLocationAtIndex(int r, int c, int TotalCol, float GridInterval, XMFLOAT3 Origin, XMFLOAT3 AxisForward, XMFLOAT3 AxisRight)
{
	XMVECTOR OriginV = XMLoadFloat3(&Origin);
	XMVECTOR ForwardV = XMLoadFloat3(&AxisForward);
	XMVECTOR RightV = XMLoadFloat3(&AxisRight);

	// 1. 전방 거리 계산 (Row 방향)
	float ForwardOffset = r * GridInterval;

	// 2. 좌우 거리 계산 (Col 방향 - 중앙 정렬 적용)
	// 전체 열 개수(TotalCol)를 알아야 중앙 위치를 잡을 수 있습니다.
	float RightOffset = (c - (TotalCol - 1) * 0.5f) * GridInterval;

	// 3. 최종 위치 계산
	XMVECTOR PosV = OriginV + (ForwardV * ForwardOffset) + (RightV * RightOffset);

	XMFLOAT3 OutPos;
	XMStoreFloat3(&OutPos, PosV);
	return OutPos;
}

void AMissileSwarmSystem::SetTargetMarkersLocation(XMFLOAT3 Origin, XMFLOAT3 Right, float GridInterval)
{
	// mTargetMarkers도 std::vector<std::vector<...>> 형태
	int Row = (int)mTargetMarkers.size();
	if (Row == 0) return;
	int Col = (int)mTargetMarkers[0].size();

	WWorld* World = GetWorld();

	// 1. 방향 벡터 직교화 (안전하게 변수에 담아 주소 전달)
	XMVECTOR WorldUpV = XMVectorSet(0, 1, 0, 0);
	XMVECTOR TempRightV = XMLoadFloat3(&Right);

	XMVECTOR ForwardV = XMVector3Normalize(XMVector3Cross(TempRightV, WorldUpV));
	XMVECTOR RightV = XMVector3Cross(WorldUpV, ForwardV);

	XMFLOAT3 FinalForward, FinalRight;
	XMStoreFloat3(&FinalForward, ForwardV);
	XMStoreFloat3(&FinalRight, RightV);

	// 2. 2차원 벡터 데이터 생성
	std::vector<std::vector<XMFLOAT3>> GridPositionOrigins;
	CalcGridLocation(Row, Col, GridInterval, Origin, FinalForward, FinalRight, GridPositionOrigins);

	// 3. 마커에 적용
	FTransform Transform;
	Transform.Rotation = FDXMath::GetEulerRotationFromVectors(ForwardV, RightV, WorldUpV);

	for (int r = 0; r < Row; ++r)
	{
		for (int c = 0; c < Col; ++c)
		{
			// std::weak_ptr나 raw pointer라고 가정하고 lock() 혹은 체크 후 사용
			if (auto Marker = mTargetMarkers[r][c].lock())
			{
				XMFLOAT3 TraceStart = GridPositionOrigins[r][c];
				TraceStart.y += 2;
				XMFLOAT3 TraceEnd = GridPositionOrigins[r][c];
				TraceEnd.y -= 5;
				TArray<AActor*> ActorsToIgnore;
				FHitResult Hit;
				World->LineTrace(TraceStart, TraceEnd, ActorsToIgnore, Hit, true, 0);
				if(Hit.Actor.expired())
				{
					Transform.Translation = TraceEnd;
				}
				else
				{
					Transform.Translation = Hit.ImpactPoint;
				}
				
				Marker->SetActorTransform(Transform);
			}
		}
	}
}

void AMissileSwarmSystem::Fire()
{
	if (mTargetMarkers.size() == 0 || mTargetMarkers[0].size() == 0)
	{
		return;
	}

	mbIsLaunching = true;
	mLastFiredRow = -1;
	mElapsedTime = 0.0f;

	mMissileGridManager = GetWorld()->SpawnActor<AMissileGridManager>();
	if (auto Manager = mMissileGridManager.lock())
	{
		Manager->SetMissileCounting((int)(mTargetMarkers.size() * mTargetMarkers[0].size()));
	}
}

void AMissileSwarmSystem::CreateHomingPaths(TWeakPtr<AActor> Target, TArray<TWeakPtr<AActor>>& HomingPaths)
{
	if (auto TargetMarker = Target.lock())
	{
		XMFLOAT3 CurrPos = GetActorLocation();
		XMFLOAT3 TargetPos = TargetMarker->GetActorLocation();
		XMVECTOR TargetPosV = XMLoadFloat3(&TargetPos);
		XMVECTOR CurrPosV = XMLoadFloat3(&CurrPos);
		XMVECTOR ToTargetV = XMVectorSubtract(TargetPosV, CurrPosV);
		float Dist = XMVectorGetX(XMVector3Length(ToTargetV));

		XMFLOAT3 TraceStart = TargetPos;
		XMVECTOR TraceStartV = XMLoadFloat3(&TraceStart);
		TraceStart.y += 0.1f;
		XMVECTOR ToTargetN = XMVector3Normalize(ToTargetV);
		XMVECTOR UpN = XMVectorSet(0, 1, 0, 0);
		float Radian = XMVectorGetX(XMVector3AngleBetweenNormals(UpN, ToTargetN));
		if (XMConvertToRadians(90) <= Radian && Radian < 175)
		{
			XMVECTOR RightN = XMVector3Normalize(XMVector3Cross(UpN, ToTargetN));
			XMVECTOR ForwardN = XMVector3Normalize(XMVector3Cross(RightN, UpN));

			XMVECTOR TraceEndV = XMVectorAdd(
				TraceStartV,
				XMVectorAdd(
					XMVectorMultiply(
						RightN,
						XMVectorReplicate(FDXMath::RandF(mMinHomingPathOffset.x, mMaxHomingPathOffset.x))
					),
					XMVectorAdd(
						XMVectorMultiply(
							UpN,
							XMVectorReplicate(FDXMath::Clamp(Dist / 2.0f, mMinHomingPathOffset.y, mMaxHomingPathOffset.y))
						),
						XMVectorMultiply(
							ForwardN,
							XMVectorReplicate(FDXMath::RandF(mMinHomingPathOffset.z, mMaxHomingPathOffset.z))
						)
					)
				)
			);
			XMFLOAT3 TraceEnd;
			XMStoreFloat3(&TraceEnd, TraceEndV);
			FHitResult HitResult;

			TArray<AActor*> ActorsToIgnore;
			GetWorld()->LineTrace(TraceStart, TraceEnd, ActorsToIgnore, HitResult, true, 1.0f);

			if (HitResult.HitComponent.expired())
			{
				FActorSpawnParameter Param;
				Param.Transform.Translation = TraceEnd;
				TWeakPtr<AActor> Path = GetWorld()->SpawnActor<AActor>(Param);
				HomingPaths.push_back(Path);
			}
		}
	}

	HomingPaths.push_back(Target);
}
