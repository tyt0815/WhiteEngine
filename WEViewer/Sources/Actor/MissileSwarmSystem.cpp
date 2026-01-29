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
	int TargetIndex = min((int)mFireInfos.size() - 1, (int)(mElapsedTime / mFireDelay));
	auto HitManager = mHitManager.lock();
	for (int r = mLastFiredRow + 1; r <= TargetIndex; ++r)
	{
		for (auto FireInfo : mFireInfos[r])
		{
			if (TSharedPtr<AColdLaunchAnimPlayer> AnimPlayer = FireInfo.AnimPlayer.lock())
			{
				FActorSpawnParameter Param;
				Param.Transform = AnimPlayer->GetActorTransform();
				if (auto Missile = GetWorld()->SpawnActor<ATopAttackMissile>(Param).lock())
				{
					Missile->SetHitManager(mHitManager);
					AnimPlayer->PlayAnim(Missile, FireInfo.TargetPos);
				}
			}
		}
	}
	mLastFiredRow = TargetIndex;

	if (mLastFiredRow + 1 >= mFireInfos.size())
	{
		mbIsLaunching = false;
		mHitManager.reset();
	}
		
}

void AMissileSwarmSystem::CreateTargetMarkers(int Row, int Col, float GridInterval)
{
	mTargetMarker = GetWorld()->SpawnActor<ATargetMarker>();
	auto Marker = mTargetMarker.lock();
	Marker->CreateTargetMarkers(Row, Col, GridInterval);
}

void AMissileSwarmSystem::SetTargetMarkerTransform(FTransform Transform)
{
	if (auto TargetMarker = mTargetMarker.lock())
	{
		TargetMarker->SetActorTransform(Transform);
	}
}

void AMissileSwarmSystem::Fire(int Row, int Col, XMFLOAT3 TargetOrigin)
{
	if (Row == 0 || Col == 0)
	{
		return;
	}

	if (mbIsLaunching)
	{
		return;
	}

	mbIsLaunching = true;

	mElapsedTime = 0.0f;
	mLastFiredRow = -1;

	mHitManager = GetWorld()->SpawnActor<AHitManager>();

	XMFLOAT3 SystemOrigin = GetActorLocation();
	XMVECTOR SystemOriginV = XMLoadFloat3(&SystemOrigin);
	XMFLOAT3 Forward = GetForwardVector();
	XMFLOAT3 Right = GetRightVector();
	XMFLOAT3 Up = GetUpVector();
	XMVECTOR ForwardV = XMLoadFloat3(&Forward);
	XMVECTOR RightV = XMLoadFloat3(&Right);
	XMVECTOR UpV = XMLoadFloat3(&Up);

	XMVECTOR TargetOriginV = XMLoadFloat3(&TargetOrigin);

	float LaunchPosGap = .2f;
	float TargetPosGap = 2;	// 간격

	float halfRow = (Row - 1) * 0.5f;
	float halfCol = (Col - 1) * 0.5f;

	mFireInfos.resize(Row);
	for (int r = 0; r < Row; ++r)
	{
		float RowPos = (float)r - halfRow;
		mFireInfos[r].resize(Col);
		for (int c = 0; c < Col; ++c)
		{
			float ColPos = (float)c - halfCol;

			XMVECTOR LaunchPosV = SystemOriginV - (UpV * RowPos * LaunchPosGap) + (RightV * ColPos * LaunchPosGap);
			XMVECTOR TargetPosV = TargetOriginV + (ForwardV * RowPos * TargetPosGap) + (RightV * ColPos * TargetPosGap);

			XMFLOAT3 LaunchPos;
			XMStoreFloat3(&LaunchPos, LaunchPosV);
			XMStoreFloat3(&mFireInfos[r][c].TargetPos, TargetPosV);

			mFireInfos[r][c].AnimPlayer = GetWorld()->SpawnActor<AColdLaunchAnimPlayer>();
			if (TSharedPtr<AColdLaunchAnimPlayer> AnimPlayer = mFireInfos[r][c].AnimPlayer.lock())
			{
				AnimPlayer->SetActorLocation(LaunchPos);
				AnimPlayer->SetActorRotation(GetActorRotation());
			}
		}
	}
}
