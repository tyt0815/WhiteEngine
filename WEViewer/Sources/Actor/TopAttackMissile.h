#pragma once

#include "Actor/StateMachineActor.h"
#include "Physics/HitResult.h"

class AMissileGridManager;

class ATopAttackMissile : public AStateMachineActor
{
	typedef AStateMachineActor Super;
public:
	ATopAttackMissile();

	virtual void OnDestroy() override;

public:
	void Initialize(WSceneComponent* HomingTarget);

private:
	void OnHit(const FHitResult& Hit);

	AMissileGridManager* mMissileGridManager = nullptr;
};

REGISTER_ACTOR(ATopAttackMissile)