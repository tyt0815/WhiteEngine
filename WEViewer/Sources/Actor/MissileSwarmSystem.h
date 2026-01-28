#pragma once
#include "Actor/Actor.h"
#include "ColdLaunchAnimPlayer.h"
#include "TopAttackMissile.h"

class AMissileSwarmSystem : public AActor
{
	typedef AActor Super;

	struct FFireInfo
	{
		TWeakPtr<AColdLaunchAnimPlayer> AnimPlayer;
		TWeakPtr<ATopAttackMissile> Missile;
		XMFLOAT3 TargetPos;
	};
public:
	AMissileSwarmSystem();

	virtual void Tick(float Delta) override;

public:
	void Fire(int Row, int Col, XMFLOAT3 TargetPos);

private:
	TArray<TArray<FFireInfo>> mFireInfos;

	float mFireDelay = 0.1f;

	float mElapsedTime = 0.0f;

	int mLastFiredRow = -1;
};
