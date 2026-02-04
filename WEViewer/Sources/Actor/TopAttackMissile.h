#pragma once

#include "Actor/ProjectileBase.h"
#include "Physics/HitResult.h"

class AMissileGridManager;

class ATopAttackMissile : public AProjectileBase
{
	typedef AProjectileBase Super;
public:
	ATopAttackMissile();

	virtual void Tick(float DeltaSecond) override;

	virtual void BeginPlay() override;

	virtual void OnDestroy() override;

public:
	void Initialize(const TArray<TWeakPtr<AActor>>& HomingPaths, AMissileGridManager* HitManager);

	void NextHomingPath();

	// mHomingPathMarkerDeque의 front에 있는 Marker를 현재 목표지점으로 설정
	void UpdateHomingPath();

	TWeakPtr<AActor> GetCurrentHomingTarget() const;

private:
	void OnHit(const FHitResult& Hit);

	TWeakPtr<WProjectileMovementComponent> mProjectileMovementComponent;

	TWeakPtr<WStaticMeshComponent> mStaticMesh;

	AMissileGridManager* mMissileGridManager = nullptr;

	// 호밍 타겟이 설정된 지점부터, 호밍 타겟까지의 거리. 실제로 이동할 거리와는 오차가 있음
	float mExpectedHomingDistanceSq;

	// 호밍 타겟. 데크의 front에 있는 액터를 향해서 날아간다.
	TDeque<TWeakPtr<AActor>> mHomingPathMarkerDeque;

	// 호밍 타겟의 해당 변수의 반경에 진입했을 때, 호밍 타겟을 제거하기 위한 값
	float mArrivalThresholdSq;	

	float mMinArrivalThresholdSq = 10;

	float mMaxArrivalThresholdSq = 25;

	// 틱당 액터에 더해질 RotationZ.
	float mRotationZStep = 0.0f;

	float mMinRotationZStep = 0;

	float mMaxRotationZStep = 0;

	XMFLOAT3 mLastTickLocation;

	friend class AMissileGridManager;
};

REGISTER_ACTOR(ATopAttackMissile)