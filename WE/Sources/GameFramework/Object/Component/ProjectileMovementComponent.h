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

	void AddForce(const XMFLOAT3& Force);

protected:
	XMFLOAT3 mInitialVelocity = { 0, 0, 0 };

	float mMaxSpeed = 0.0f;		// 0일 경우 제한x

	float mAcceleration = 0.0f; // 전방 추진 가속도 (스칼라)

	float mGravityScale = 1.0f; // 중력 배율 (기본 9.8m/s^2에 곱해질 값)

	// 투사체 수명. 0일시 영구 지속
	float mLifeSpan = 5.0f;

	float mHomingTurnLimit = 0;

	bool mbHomingProjectile = false;

private:
	TWeakPtr<WSceneComponent> mHomingTarget;

	XMFLOAT3 mExternalAcceleration = { 0,0,0 };

	float mLifeTimeElapsed = 0.0f;

public:
	__forceinline const XMFLOAT3& GetInitialVelocity() const { return mInitialVelocity; }
	__forceinline void SetInitialVelocity(const XMFLOAT3& Value) { mInitialVelocity = Value; }

	// --- Max Speed ---
	__forceinline float GetMaxSpeed() const { return mMaxSpeed; }
	__forceinline void SetMaxSpeed(float Value) { mMaxSpeed = Value; }

	// --- Acceleration ---
	__forceinline float GetAcceleration() const { return mAcceleration; }
	__forceinline void SetAcceleration(float Value) { mAcceleration = Value; }

	// --- Gravity Scale ---
	__forceinline float GetGravityScale() const { return mGravityScale; }
	__forceinline void SetGravityScale(float Value) { mGravityScale = Value; }

	// --- Life Span ---
	__forceinline float GetLifeSpan() const { return mLifeSpan; }
	__forceinline void SetLifeSpan(float Value) { mLifeSpan = Value; }

	// --- Homing Properties ---
	__forceinline bool IsHoming() const { return mbHomingProjectile; }
	__forceinline void SetHoming(bool bEnabled) { mbHomingProjectile = bEnabled; }

	__forceinline float GetHomingTurnLimit() const { return mHomingTurnLimit; }
	__forceinline void SetHomingTurnLimit(float Value) { mHomingTurnLimit = Value; }

	inline WSceneComponent* GetHomingTarget() const
	{
		TSharedPtr<WSceneComponent> Target = mHomingTarget.lock();
		return Target ? Target.get() : nullptr;
	}
};

REGISTER_COMPONENT(WProjectileMovementComponent);