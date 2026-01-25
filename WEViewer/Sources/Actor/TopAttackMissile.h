#pragma once

#include "Actor/Actor.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/StaticMeshComponent.h"

class ATopAttackMissile : public AActor
{
	typedef AActor Super;
public:
	ATopAttackMissile();

	void SetTargetPosition(XMFLOAT3 Pos);

	virtual void OnDestroy() override;

	virtual void Tick(float DeltaSecond) override;

private:
	TWeakPtr<WProjectileMovementComponent> mProjectileMovementComponent;

	TWeakPtr<WStaticMeshComponent> mStaticMesh;

	TWeakPtr<AActor> mTargetMarker;
};