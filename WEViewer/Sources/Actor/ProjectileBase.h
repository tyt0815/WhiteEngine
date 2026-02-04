#pragma once
#include "Actor/Actor.h"
#include "Component/BoxComponent.h"
#include "Component/ProjectileMovementComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/ObjectAnimComponent.h"

class AProjectileBase : public AActor
{
	typedef AActor Super;
public:
	AProjectileBase();

	virtual void Tick(float DeltaSecond) override;

	virtual void BeginPlay() override;

protected:
	void SetSmartHoming(bool bSmartHoming, float Range);

private:
	TWeakPtr<WProjectileMovementComponent> mProjMoveComp;
	bool mbSmartHoming;
	float mSmartHomingRange;
};

REGISTER_ACTOR(AProjectileBase);