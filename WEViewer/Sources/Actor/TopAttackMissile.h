#pragma once

#include "Actor/Actor.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/BoxComponent.h"
#include "Component/ObjectAnimComponent.h"

class ATopAttackMissile : public AActor
{
	typedef AActor Super;
public:
	ATopAttackMissile();

	void SetTargetPosition(XMFLOAT3 Pos);

	virtual void OnDestroy() override;

	virtual void Tick(float DeltaSecond) override;

public:
	void PushFrontHomingPath(XMFLOAT3 Pos);

	void PushBackHomingPath(XMFLOAT3 Pos);

	void NextHomingPath();

	// mHomingPathMarkerDeque의 front에 있는 Marker를 현재 목표지점으로 설정
	void UpdateHomingPath();

	TWeakPtr<AActor> GetCurrentHomingTarget() const;

private:
	void DestroyPathMarkers();

	TWeakPtr<WBoxComponent> mHitBoxComp;

	TWeakPtr<WProjectileMovementComponent> mProjectileMovementComponent;

	TWeakPtr<WStaticMeshComponent> mStaticMesh;

	TWeakPtr<WObjectAnimComponent> mObjAnimComp;

	TArray<FObjectAnimSampler*> mMissileAnimSamplers;

	FObjectAnimSampler* mCurrAnimSampler;

	float mAnimElapsedTime;

	// 호밍 타겟이 설정된 지점부터, 호밍 타겟까지의 거리. 실제로 이동할 거리와는 오차가 있음
	float mExpectedHomingDistanceSq;

	// 미사일 애니메이션의 마지막 프레임
	float mAnimFrameEnd = 0;

	// 호밍 타겟. 데크의 front에 있는 액터를 향해서 날아간다.
	TDeque<TWeakPtr<AActor>> mHomingPathMarkerDeque;

	float mMaxAltitude = 30;		// 탑 어택시 경유할 목표지점의 높이

	float mArrivalThresholdSq = 100.0f;	// 호밍 패스를 경유할 때, 해당 지점에 도달했는지 확인하기 위한 거리 offset
};