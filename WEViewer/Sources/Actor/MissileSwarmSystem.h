#pragma once
#include "Actor/Actor.h"
#include "ColdLaunchAnimPlayer.h"
#include "TargetMarker.h"
#include "MissileGridManager.h"

class AMissileSwarmSystem : public AActor
{
	typedef AActor Super;
public:
	AMissileSwarmSystem();

	virtual void Tick(float Delta) override;

public:
	void ClearTargetMarkers();

	void CreateTargetMarkers(int Row, int Col);

	void CalcGridLocation(int Row, int Col, XMFLOAT3 Origin, XMFLOAT3 AxisX, XMFLOAT3 AxisY, TArray<XMFLOAT3>& Locations);

	void SetTargetMarkersLocation(XMFLOAT3 Origin, XMFLOAT3 Forward, XMFLOAT3 Right, float GridInterval);

	void Fire();

private:
	void CreateHomingPaths(TWeakPtr<AActor> Target, TArray<TWeakPtr<AActor>>& HomingPaths);

	TArray<TArray<TWeakPtr<ATargetMarker>>> mTargetMarkers;

	TWeakPtr<AMissileGridManager> mHitManager;

	float mFireDelay = 0.2f;

	float mElapsedTime = 0.0f;

	int mLastFiredRow = -1;

	bool mbIsLaunching = false;

	const XMFLOAT3 mMinHomingPathOffset = { -0.5f, 5, -6 };

	const XMFLOAT3 mMaxHomingPathOffset = { 0.5f ,25, -5.5f };
};
