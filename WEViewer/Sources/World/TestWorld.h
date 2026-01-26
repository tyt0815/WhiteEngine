#pragma once

#include "GameFramework/Object/World/DefaultWorld.h"
#include "Actor/MissileSwarmSystem.h"

class WTestWorld : public WDefaultWorld
{
	typedef WDefaultWorld Super;
public:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSecond) override;

private:
	XMFLOAT3 CalcTargetOrigin(AActor* Actor, float TargetDistance);

	float mDelay = 2;

	float mElapsedTime;

	TWeakPtr<AMissileSwarmSystem> mMissileSystems[2];

	TWeakPtr<AActor> mPlatform;
};