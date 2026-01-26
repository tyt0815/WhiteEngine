#pragma once

#include "Actor/Actor.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/BoxComponent.h"

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

	TDeque<TWeakPtr<AActor>> mHomingPathMarkerDeque;

	float mMaxAltitude = 30;		// 탑 어택시 경유할 목표지점의 높이

	float mArrivalThresholdSq = 100.0f;	// 호밍 패스를 경유할 때, 해당 지점에 도달했는지 확인하기 위한 거리 offset
};