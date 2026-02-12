#pragma once
#include "MovementComponent.h"
#include "Physics/HitResult.h"
#include "SceneComponent.h"

DECLARE_MULTICAST_DELEGATE(FOnLockon);
DECLARE_MULTICAST_DELEGATE(FOnBounce);

enum class EHomingStrategy : uint8_t
{
	Nearest,    // 가장 가까운 대상
	Angle,       // 정면 각도가 가장 일치하는 대상
	None,
};

class WProjectileMovementComponent : public WMovementComponent
{
	typedef WMovementComponent Super;
public:
	WProjectileMovementComponent();

	virtual void Tick(float DeltaTime) override;

	virtual void BeginComponent() override;

public:
	void SetHomingTarget(WSceneComponent* Target);

	void SetHomingLocation(const XMFLOAT3& Loc);

	void AddForce(const XMFLOAT3& Force);

	void BindCollisionEvent(class FCollisionGeneratorBase* CollisionGenerator);

	FOnLockon mOnLockon;

	FOnBounce mOnBounce;

protected:
	XMFLOAT3 mInitialVelocity = { 0, 0, 0 };

	float mMaxSpeed = 0.0f;		// 0일 경우 제한x

	float mAcceleration = 0.0f; // 전방 추진 가속도 (스칼라)

	float mGravityScale = 0; // 중력 배율 (기본 9.8m/s^2에 곱해질 값)

	// 투사체 수명. 0일시 영구 지속
	float mLifeSpan = 5.0f;

	float mHomingTurnLimit = 0;

	bool mbHomingProjectile = false;

	bool mbOrientRotationToMovement = true;

private:
	void OnCollision(WSceneComponent* Instigator, WPhysicsComponent* HittedComponent, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance, float Damage);

	void Bounce_Internal(XMFLOAT3 Normal);

	TWeakPtr<WSceneComponent> mHomingTarget;

	XMFLOAT3 mHomingLocation;

	XMFLOAT3 mExternalAcceleration = { 0,0,0 };

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
	EHomingStrategy mHomingStrategy = EHomingStrategy::None;

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

	friend class AProjectileBase;
};

REGISTER_COMPONENT(WProjectileMovementComponent);