#pragma once
#include "Actor/Actor.h"
#include "Physics/HitResult.h"
#include "Component/BoxComponent.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/ObjectAnimComponent.h"
#include "Component/SplineComponent.h"

enum class EHomingStrategy : uint8_t
{
	Nearest,    // 가장 가까운 대상
	Angle,       // 정면 각도가 가장 일치하는 대상
	None,
};

class AProjectileBase : public AActor
{
	typedef AActor Super;

public:
	AProjectileBase();

	virtual void Tick(float DeltaSecond) override;

	virtual void BeginPlay() override;

protected:
	virtual void LoadWConfigs(const std::unordered_map<std::string, WAttributesMap>& Configs) override;

	virtual void ApplyWComponentCommonAttribute(struct FBlueprintComponentNode* CompNode, WSceneComponent* Comp) override;

protected:
	void PlayParticle(const std::string& Name);

private:
	void OnCollision(AActor* Actor, WPhysicsComponent* Comp, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance, float Damage);

	TWeakPtr<WProjectileMovementComponent> mProjMoveComp;
	TWeakPtr<WObjectAnimComponent> mObjAnimComp;

	std::unordered_map<std::string, class WObjectAnimComponent*> mWObjAnimComp;

	TArray<WEvent> mOnHitEvents;
	WEvent mCommonOnHitEvent;

	// 호밍
	void SetHomingTarget(AActor* Target);

	void UpdateHoming(float DeltaSecond);

	AActor* FindBestHomingTarget();

	AActor* FindHomingTarget_Nearest(const TArray<FHitResult>& Hits);

	AActor* FindHomingTarget_Angle(const TArray<FHitResult>& Hits);

	bool IsHomingTarget(AActor* Actor) const;


	std::set<std::string> mHomingTargetTags;
	float mHomingRange = 10.0f;
	float mHomingAngle = 45.0f;     // Angle 전략용 (Degree)
	float mRetargetTick = 0.0f;    // 타겟 갱신 주기
	float mRetargetTimer = 0.0f;   // 타이머 카운트
	EHomingStrategy mHomingStrategy = EHomingStrategy::None;

	float mHomingStopRange = 0.0f;     // 0이면 무한 호밍
	bool mbForgetPreviousTarget = true;

	std::set<AActor*> mVisitedTargets;     // 이미 호밍했던 타겟 목록
	AActor* mCurrentTarget = nullptr;   // 현재 추적 중인 타겟

	//// 호밍 경유
	void GenerateWaypoints(AActor* Target);

	bool mbUseWaypoints = false;           
	std::string mWaypointSpace = "Direction";
	std::string mWaypointBase = "Target";  // Actor or Target
	std::string mWaypointType = "Value";   // Value or Adaptive
	std::vector<XMFLOAT3> mConfigWaypoints;

	std::vector<XMFLOAT3> mFinalWaypoints;
	int mCurrentWaypointIndex = 0;
};

REGISTER_ACTOR(AProjectileBase);