#pragma once
#include "Actor/Actor.h"
#include "TopAttackMissile.h"
#include "Component/ObjectAnimComponent.h"

class AColdLaunchAnimPlayer : public AActor
{
	typedef AActor Super;
public:
	AColdLaunchAnimPlayer();

	virtual void Tick(float DeltaSecond) override;

public:
	void PlayAnim(TWeakPtr<ATopAttackMissile> Projectile, XMFLOAT3 TargetPos);

private:
	TWeakPtr<WObjectAnimComponent> mAnimComp;
	
	FObjectAnimSampler* mAnimSampler;

	TWeakPtr<ATopAttackMissile> mProjectile;

	XMFLOAT3 mTargetPos;

	float mPlayTime = 0;
};