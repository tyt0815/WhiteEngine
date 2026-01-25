#pragma once

#include "Actor/Actor.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/StaticMeshComponent.h"

class ATopAttackMissile : public AActor
{
	typedef AActor Super;
public:
	ATopAttackMissile();

	void SetTargetLocation(XMFLOAT3 Loc);

	virtual void OnDestroy() override;

private:
	TWeakPtr<WProjectileMovementComponent> mProjectileMovementComponent;

	TWeakPtr<WStaticMeshComponent> mStaticMesh;

	TWeakPtr<AActor> mTargetMarker;
};