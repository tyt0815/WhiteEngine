#pragma once
#include "PhysicsMovementComponent.h"
#include "Physics/HitResult.h"

DECLARE_MULTICAST_DELEGATE(FProjectileMovementEventDelegate);

class WProjectileMovementComponent : public WPhysicsMovementComponent
{
	typedef WPhysicsMovementComponent Super;
public:
	WProjectileMovementComponent();

	virtual void Tick(float DeltaTime) override;

	virtual void BeginComponent() override;

public:
	void SetHomingTarget(WSceneComponent* Target);

	void SetHomingLocation(const XMFLOAT3& Loc);

	void BindCollisionEvent(class FCollisionGeneratorBase* CollisionGenerator);

	FProjectileMovementEventDelegate mOnLockon;

	FProjectileMovementEventDelegate mOnBounce;

	FProjectileMovementEventDelegate mOnHomingSuccess;

	FProjectileMovementEventDelegate mOnHomingFail;

protected:

	float mAcceleration = 0.0f; // 전방 추진 가속도 (스칼라)

	// 투사체 수명. 0일시 영구 지속
	float mLifeSpan = 5.0f;

	float mHomingTurnLimit = 0;

	bool mbHomingProjectile = false;

private:
	void OnCollision(WSceneComponent* Instigator, WPhysicsComponent* HittedComponent, XMFLOAT3 ImpulseDir, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance);

	void Bounce_Internal(XMFLOAT3 Normal);

	TWeakPtr<WSceneComponent> mHomingTarget;

	XMFLOAT3 mHomingLocation;

	float mLifeTimeElapsed = 0.0f;

	bool mShouldBounce = false;
	float mBounciness = 0.6f;
	float   mMaxBounces = 3;
	int   mCurrentBounces = 0;

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////

	void UpdateHoming(float DeltaSecond);

	AActor* FindBestHomingTarget();

	AActor* FindHomingTarget_Nearest(const TArray<FHitResult>& Hits);

	AActor* FindHomingTarget_Angle(const TArray<FHitResult>& Hits);

	bool IsHomingTarget(AActor* Actor) const;

	void GenerateWaypoints(WSceneComponent* Target);

	
	std::set<std::string> mHomingTargetTags;
	float mHomingRange = 10.0f;
	float mHomingAngle = 45.0f;     // Angle 전략용 (Degree)
	float mRetargetTick = 0.0f;    // 타겟 갱신 주기
	float mRetargetTimer = 0.0f;   // 타이머 카운트
	std::string mHomingStrategy = "None";

	float mHomingStopRange = 0.0f;     // 0이면 무한 호밍
	bool mbForgetPreviousTarget = true;

	std::set<AActor*> mVisitedTargets;     // 이미 호밍했던 타겟 목록	

	bool mbUseWaypoints = false;
	std::string mWaypointSpace = "Direction";
	std::string mWaypointBase = "Target";  // Actor or Target
	std::string mWaypointType = "Value";   // Value or Adaptive
	std::vector<XMFLOAT3> mConfigWaypoints;

	std::vector<XMFLOAT3> mFinalWaypoints;

	TWeakPtr<WSceneComponent> mFinalHomingTarget;

	int mCurrentWaypointIndex = 0;

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////





public:
	__forceinline float GetMaxSpeed() const { return mMaxSpeed; }
	__forceinline void SetMaxSpeed(float Value) { mMaxSpeed = Value; }

	__forceinline float GetAcceleration() const { return mAcceleration; }
	__forceinline void SetAcceleration(float Value) { mAcceleration = Value; }

	__forceinline float GetLifeSpan() const { return mLifeSpan; }
	__forceinline void SetLifeSpan(float Value) { mLifeSpan = Value; }

	__forceinline bool IsHoming() const { return mbHomingProjectile; }
	__forceinline void SetHoming(bool bEnabled) { mbHomingProjectile = bEnabled; }

	__forceinline float GetHomingTurnLimit() const { return mHomingTurnLimit; }
	__forceinline void SetHomingTurnLimit(float Value) { mHomingTurnLimit = Value; }

	inline WSceneComponent* GetHomingTarget() const
	{
		TSharedPtr<WSceneComponent> Target = mFinalHomingTarget.lock();
		return Target ? Target.get() : nullptr;
	}

	friend class WComponentRegistry;
};

REGISTER_COMPONENT(WProjectileMovementComponent);