#pragma once
#include "Actor/Actor.h"
#include "World/World.h"
#include "Component/ObjectAnimComponent.h"
#include "TopAttackMissile.h"

class AColdLaunchAnimPlayer : public AActor
{
	typedef AActor Super;
public:
	AColdLaunchAnimPlayer();

	virtual void Tick(float DeltaSecond) override;

public:
	void PlayAnim(XMFLOAT3 TargetPos);

private:
	TWeakPtr<WObjectAnimComponent> mAnimComp;
	
	FObjectAnimSampler* mAnimSampler;

	FCurveSampler mLocationZSampler;

	FCurveSampler mRotationSampler;

	TWeakPtr<ATopAttackMissile> mProjectile;

	TWeakPtr<AActor> mCovers[4];

	XMFLOAT3 mRotationAxis;

	float mRotationRadian;

	float mPlayTime = 0;
};

