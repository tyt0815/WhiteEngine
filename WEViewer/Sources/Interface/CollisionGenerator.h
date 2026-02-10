#pragma once
#include "Utility/Delegate.h"
#include <DirectXMath.h>

class AActor;
class WPhysicsComponent;
using namespace DirectX;

// Actor, Component, ImpactPoint, Normal, Distance;
DECLARE_MULTICAST_DELEGATE_FiveParams(FOnCollision, AActor*, WPhysicsComponent*, XMFLOAT3, XMFLOAT3, float);
// DECLARE_MULTICAST_DELEGATE()

class ICollisionGenerator
{
public:
	virtual ~ICollisionGenerator() = default;

	virtual void GenerateCollision() = 0;

protected:
	virtual void ActivateCollision() = 0;

	virtual void DeactivateCollision() = 0;
};

class FCollisionGeneratorBase : public ICollisionGenerator
{
public:
	FOnCollision mOnCollision;

private:
	float mHitDelay;
	int mMaxHit;

public:
	__forceinline void SetHitDelay(float Value)
	{
		mHitDelay = Value;
	}

	__forceinline void SetMaxHit(int Value)
	{
		mMaxHit = Value;
	}
};