
#pragma once
#include "Actor/Actor.h"
#include "ColdLaunchAnimPlayer.h"



class AMissileSwarmSystem : public AActor
{
	typedef AActor Super;

	struct FFireInfo
	{
		TWeakPtr<AColdLaunchAnimPlayer> AnimPlayer;
		XMFLOAT3 TargetPos;
	};
public:
	AMissileSwarmSystem();

	virtual void Tick(float Delta) override;

public:
	template<typename TColdLaunchAnimPlayer>
	void Fire(int Row, int Col, XMFLOAT3 TargetPos);
	
private:
	TArray<TArray<FFireInfo>> mFireInfos;

	float mFireDelay = 0.1f;

	float mElapsedTime = 0.0f;

	int mLastFiredRow = -1;
};

template<typename TColdLaunchAnimPlayer>
inline void AMissileSwarmSystem::Fire(int Row, int Col, XMFLOAT3 TargetOrigin)
{
	if (Row == 0 || Col == 0)
	{
		return;
	}

	mElapsedTime = 0.0f;
	mLastFiredRow = -1;

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

			mFireInfos[r][c].AnimPlayer = GetWorld()->SpawnActor<TColdLaunchAnimPlayer>();
			if (TSharedPtr<AColdLaunchAnimPlayer> AnimPlayer = mFireInfos[r][c].AnimPlayer.lock())
			{
				AnimPlayer->SetActorLocation(LaunchPos);
				AnimPlayer->SetActorRotation(GetActorRotation());
			}
		}
	}
}
