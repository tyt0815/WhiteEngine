#pragma once
#include <DirectXMath.h>
#include "Physics/HitResult.h"
#include "Utility/Delegate.h"
#include "Utility/Container.h"
#include <set>

class AActor;
class WPhysicsComponent;
using namespace DirectX;

// void OnCollision(WSceneComponent* Instigator, WPhysicsComponent* HittedComponent, XMFLOAT3 ImpulseDir, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance)
// Instigator, HittedComponent, ImpulseDir, ImpactPoint, Normal, Distance
DECLARE_MULTICAST_DELEGATE_SixParams(FOnCollision, WSceneComponent*, WPhysicsComponent*, XMFLOAT3, XMFLOAT3, XMFLOAT3, float);

class ICollisionGenerator
{
public:
	virtual ~ICollisionGenerator() = default;

	virtual void GenerateCollision() = 0;
};

class FCollisionGeneratorBase : public ICollisionGenerator
{
public:
	FCollisionGeneratorBase();

	void AddTargetTags(const TArray<std::string>& InTargetTags);

	void AddTargetTags(const std::set<std::string>& InTargetTags);

	FOnCollision mOnCollision;

protected:
	void UpdateActorsToIgnore(float DeltaSecond);

	void UpdateCachedIgnoreList();

	void ProcessHit(const FHitResult& Hit);

	std::unordered_map<AActor*, float> mIgnoreTimers;
	std::vector<AActor*> mCachedIgnoreList;

	bool mbDebug = false;

	float mCollisionInterval = 1.0f / 60.0f;

	XMFLOAT3 mMoveDir;

private:
	std::set<std::string> mTargetTags;

	// 0일 경우 제한 없음
	float mHitDelay = 0;

	// 0일 경우 제한 없음
	float mMaxHit = 0;

	int mHitCount = 0;

public:
	__forceinline void SetHitDelay(float Value)
	{
		mHitDelay = Value;
	}

	__forceinline void SetMaxHit(int Value)
	{
		mMaxHit = (float)Value;
	}

	__forceinline void SetDebug(bool Value)
	{
		mbDebug = Value;
	}

	__forceinline bool IsTargetTag(const std::string& Tag)
	{
		return mTargetTags.find(Tag) != mTargetTags.end();
	}

	friend class WComponentRegistry;
};