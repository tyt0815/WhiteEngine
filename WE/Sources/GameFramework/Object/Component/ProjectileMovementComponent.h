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

protected:
	XMFLOAT3 mInitialVelocity = { 0, 0, 0 };

	float mMaxSpeed = 0.0f;

	float mAcceleration = 0.0f; // 전방 추진 가속도 (스칼라)

	float mGravityScale = 1.0f; // 중력 배율 (기본 9.8m/s^2에 곱해질 값)

	// 투사체 수명. 0일시 영구 지속
	float mLifeSpan = 5.0f;

	float mHomingTurnLimit = 0;

	bool mbHomingProjectile = false;

private:
	TWeakPtr<WSceneComponent> mHomingTarget;

	float mLifeTimeElapsed = 0.0f;

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

	inline WSceneComponent* GetHomingTarget() const
	{
		TSharedPtr<WSceneComponent> Target = mHomingTarget.lock();
		return Target ? Target.get() : nullptr;
	}
};

REGISTER_COMPONENT(WProjectileMovementComponent);