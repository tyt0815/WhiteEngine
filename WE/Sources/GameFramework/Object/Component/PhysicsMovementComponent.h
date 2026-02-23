#pragma once

#include "MovementComponent.h"

class WPhysicsMovementComponent : public WMovementComponent
{
	typedef WMovementComponent Super;
public:
	virtual void Tick(float DeltaSecond) override;

	virtual void BeginComponent() override;

	void AddForce(const XMFLOAT3& Force);

protected:
	XMFLOAT3 mInitialVelocity = { 0, 0, 0 };

	float mGravityScale = 0; // 중력 배율 (기본 9.8m/s^2에 곱해질 값)

	float mMaxSpeed = 0.0f;		// 0일 경우 제한x

	bool mbOrientRotationToMovement = true;

private:
	XMFLOAT3 mExternalAcceleration = { 0,0,0 };

public:
	__forceinline const XMFLOAT3& GetInitialVelocity() const { return mInitialVelocity; }
	__forceinline void SetInitialVelocity(const XMFLOAT3& Value) { mInitialVelocity = Value; }

	__forceinline float GetGravityScale() const { return mGravityScale; }
	__forceinline void SetGravityScale(float Value) { mGravityScale = Value; }
};