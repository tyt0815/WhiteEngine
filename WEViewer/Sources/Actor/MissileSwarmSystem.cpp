#include "MissileSwarmSystem.h"
#include "World/World.h"

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
	auto HitManager = mHitManager.lock();
	int Row = (int)mTargetMarkers.size();
	int Col = (int)mTargetMarkers[0].size();

	for (int r = mLastFiredRow + 1; r <= TargetIndex; ++r)
	{
		for (auto WeakPtr : mTargetMarkers[r])
		{
			if (auto Marker = WeakPtr.lock())
			{
				auto AnimPlayer = GetWorld()->SpawnActor<AColdLaunchAnimPlayer>().lock();
				AnimPlayer->SetActorTransform(GetActorTransform());
				FActorSpawnParameter Param;
				Param.Transform = AnimPlayer->GetActorTransform();
				if (auto Missile = GetWorld()->SpawnActor<ATopAttackMissile>(Param).lock())
				{
					TArray<TWeakPtr<AActor>> HomingPaths;
					CreateHomingPaths(WeakPtr, HomingPaths);

					Missile->Initialize(HomingPaths, mHitManager.lock().get());
					
					AnimPlayer->PlayAnim(Missile);
				}
			}
		}
	}

	mLastFiredRow = TargetIndex;

	if (mLastFiredRow + 1 >= mTargetMarkers.size())
	{
		mbIsLaunching = false;
		mHitManager.reset();
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

void AMissileSwarmSystem::CreateTargetMarkers(int Row, int Col)
{
	mTargetMarkers.resize(Row);
	for (int r = 0; r < Row; ++r)
	{
		mTargetMarkers[r].resize(Col);
		for (int c = 0; c < Col; ++c)
		{
			mTargetMarkers[r][c] = GetWorld()->SpawnActor<ATargetMarker>();
		}
	}
}

void AMissileSwarmSystem::CalcGridLocation(int Row, int Col, XMFLOAT3 Origin, XMFLOAT3 AxisX, XMFLOAT3 AxisY, TArray<XMFLOAT3>& Locations)
{
}

void AMissileSwarmSystem::SetTargetMarkersLocation(XMFLOAT3 Origin, XMFLOAT3 Forward, XMFLOAT3 Right, float GridInterval)
{
	int Row = (int)mTargetMarkers.size();
	if (Row == 0)
	{
		return;
	}
	int Col = (int)mTargetMarkers[0].size();
	if (Col == 0)
	{
		return;
	}

	XMVECTOR OriginV = XMLoadFloat3(&Origin);
	XMVECTOR WorldUpV = XMVectorSet(0, 1, 0, 0);
	XMVECTOR RightV = XMLoadFloat3(&Right);
	XMVECTOR ForwardV = XMVector3Cross(RightV, WorldUpV);
	RightV = XMVector3Cross(WorldUpV, ForwardV);

	FTransform Transform;
	Transform.Rotation = FDXMath::GetEulerRotationFromVectors(ForwardV, RightV, WorldUpV);

	for (int r = 0; r < Row; ++r)
	{
		float ForwardOffset = r * GridInterval;

		for (int c = 0; c < Col; ++c)
		{
			if (auto Marker = mTargetMarkers[r][c].lock())
			{
				float RightOffset = (c - (Col - 1) * 0.5f) * GridInterval;

				XMVECTOR PosV = OriginV + (ForwardV * ForwardOffset) + (RightV * RightOffset);

				XMStoreFloat3(&Transform.Translation, PosV);
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
	mHitManager = GetWorld()->SpawnActor<AMissileGridManager>();
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
