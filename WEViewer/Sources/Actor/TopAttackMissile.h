#pragma once

#include "Actor/Actor.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/BoxComponent.h"
#include "Component/ObjectAnimComponent.h"
#include "Actor/HitManager.h"
#include "Physics/HitResult.h"

class ATopAttackMissile : public AActor
{
	typedef AActor Super;
public:
	ATopAttackMissile();

	virtual void OnDestroy() override;

	virtual void Tick(float DeltaSecond) override;

	virtual void BeginPlay() override;

public:
	void SetTargetPosition(XMFLOAT3 Pos);

	void PushFrontHomingPath(XMFLOAT3 Pos);

	void PushBackHomingPath(XMFLOAT3 Pos);

	void NextHomingPath();

	// mHomingPathMarkerDeque의 front에 있는 Marker를 현재 목표지점으로 설정
	void UpdateHomingPath();

	TWeakPtr<AActor> GetCurrentHomingTarget() const;

	void SetHitManager(TWeakPtr<AHitManager> HitManager);

private:
	void DestroyPathMarkers();

	void OnHit(const FHitResult& Hit);

	TWeakPtr<WProjectileMovementComponent> mProjectileMovementComponent;

	TWeakPtr<WStaticMeshComponent> mStaticMesh;

	TWeakPtr<WObjectAnimComponent> mObjAnimComp;

	TWeakPtr<AHitManager> mHitManager;

	TArray<FObjectAnimSampler*> mMissileAnimSamplers;

	FObjectAnimSampler* mCurrAnimSampler = nullptr;

	float mAnimElapsedTime;

	// 호밍 타겟이 설정된 지점부터, 호밍 타겟까지의 거리. 실제로 이동할 거리와는 오차가 있음
	float mExpectedHomingDistanceSq;

	// 미사일 애니메이션의 마지막 프레임
	float mAnimFrameEnd = 0;

	// 호밍 타겟. 데크의 front에 있는 액터를 향해서 날아간다.
	TDeque<TWeakPtr<AActor>> mHomingPathMarkerDeque;

	const XMFLOAT3 mMinHomingPathOffset = { -0.5f, 5, -6 };

	const XMFLOAT3 mMaxHomingPathOffset = { 0.5f ,25, -5.5f };

	// 호밍 타겟의 해당 변수의 반경에 진입했을 때, 호밍 타겟을 제거하기 위한 값
	float mArrivalThresholdSq;	

	const float mMinArrivalThresholdSq = 1.0f;

	const float mMaxArrivalThresholdSq = 100.0f;

	// 틱당 액터에 더해질 RotationZ. UpdateHomingPath를 호출할때 랜덤으로 설정된다.
	float mRotationZStep = 0.0f;

	const float mMinRotationZStep = -45.f;

	const float mMaxRotationZStep = 45.0f;

	XMFLOAT3 mLastTickLocation;
};