#pragma once
#include <DirectXMath.h>
#include "Physics/HitResult.h"
#include "Utility/Delegate.h"
#include "Utility/Container.h"
#include <set>

class AActor;
class WPhysicsComponent;
using namespace DirectX;

// Actor, Component, ImpactPoint, Normal, Distance, Damage;
DECLARE_MULTICAST_DELEGATE_SixParams(FOnCollision, AActor*, WPhysicsComponent*, XMFLOAT3, XMFLOAT3, float, float);

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
	void AddTargetTags(const TArray<std::string>& InTargetTags);

	FOnCollision mOnCollision;

protected:
	void UpdateActorsToIgnore(float DeltaSecond);

	void UpdateCachedIgnoreList();

	void ProcessHit(const FHitResult& Hit);


	std::unordered_map<AActor*, float> mIgnoreTimers;
	std::vector<AActor*> mCachedIgnoreList;

	bool mbDebug = false;

private:
	std::set<std::string> mTargetTags;

	// 0일 경우 제한 없음
	float mHitDelay = 0;

	// 0일 경우 제한 없음
	int mMaxHit = 0;

	int mHitCount = 0;

	float mDamage = 0;

public:
	__forceinline void SetHitDelay(float Value)
	{
		mHitDelay = Value;
	}

	__forceinline void SetMaxHit(int Value)
	{
		mMaxHit = Value;
	}

	__forceinline void SetDamage(float Value)
	{
		mDamage = Value;
	}

	__forceinline void SetDebug(bool Value)
	{
		mbDebug = Value;
	}

	__forceinline bool IsTargetTag(const std::string& Tag)
	{
		return mTargetTags.find(Tag) != mTargetTags.end();
	}
};