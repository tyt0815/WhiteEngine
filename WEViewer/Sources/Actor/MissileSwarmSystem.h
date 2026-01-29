#pragma once
#include "Actor/Actor.h"
#include "ColdLaunchAnimPlayer.h"
#include "TargetMarker.h"
#include "HitManager.h"

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
	void CreateTargetMarkers(int Row, int Col, float GridInterval);

	void SetTargetMarkerTransform(FTransform Transform);

	void Fire(int Row, int Col, XMFLOAT3 TargetPos);

private:
	TWeakPtr<ATargetMarker> mTargetMarker;

	TArray<TArray<FFireInfo>> mFireInfos;

	TWeakPtr<AHitManager> mHitManager;

	float mFireDelay = 0.1f;

	float mElapsedTime = 0.0f;

	int mLastFiredRow = -1;

	bool mbIsLaunching = false;
};
