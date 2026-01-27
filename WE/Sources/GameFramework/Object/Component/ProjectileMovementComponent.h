#pragma once
#include "MovementComponent.h"
#include "SceneComponent.h"

class WProjectileMovementComponent : public WMovementComponent
{
	typedef WMovementComponent Super;
public:
	virtual void Tick(float DeltaTime) override;

protected:
	// 투사체 수명. 0일시 영구 지속
	float mLifeSpan = 5.0f;

private:
	TWeakPtr<WSceneComponent> mHomingTarget;

	float mLifeTimeElapsed = 0.0f;
	
	// 호밍할때 초당 꺾일 수 있는 최대 각
	// 0일 경우, 제한x
	float mHomingTurnLimit = 0;

	bool mbHomingProjectile = false;

public:
	void SetLifeSpan(float LifeSpan)
	{
		mLifeSpan = LifeSpan;
	}

	void SetHoming(bool bHoming)
	{
		mbHomingProjectile = bHoming;
	}

	void SetHomingTurnLimit(float Value)
	{
		mHomingTurnLimit = Value;
	}

	void SetHomingTarget(TWeakPtr<WSceneComponent> Target)
	{
		mHomingTarget = Target;
	}
};