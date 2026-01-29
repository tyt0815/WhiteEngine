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
	void PlayAnim(TWeakPtr<ATopAttackMissile> Projectile);

private:
	TWeakPtr<WObjectAnimComponent> mAnimComp;
	
	FObjectAnimSampler* mAnimSampler;

	TWeakPtr<ATopAttackMissile> mProjectile;

	float mPlayTime = 0;
};