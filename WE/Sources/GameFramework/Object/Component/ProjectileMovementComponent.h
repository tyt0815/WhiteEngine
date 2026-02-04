#pragma once
#include "MovementComponent.h"
#include "SceneComponent.h"

class WProjectileMovementComponent : public WMovementComponent
{
	typedef WMovementComponent Super;
public:
	WProjectileMovementComponent();

	virtual void Tick(float DeltaTime) override;

	virtual void BeginComponent() override;

public:
	void SetHomingTarget(WSceneComponent* Target);

	void SetSpeed(float Value);

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

	float mSpeed = 1;

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

	__forceinline float GetSpeed() const
	{
		return mSpeed;
	}

	inline WSceneComponent* GetHomingTarget() const
	{
		TSharedPtr<WSceneComponent> Target = mHomingTarget.lock();
		return Target ? Target.get() : nullptr;
	}
};

REGISTER_COMPONENT(WProjectileMovementComponent);