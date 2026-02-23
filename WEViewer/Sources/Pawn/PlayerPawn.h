#pragma once
#include "Pawn/GhostCameraPawn.h"
#include "Actor/MissileSwarmSystem.h"

class IInteractionInterface;

class APlayerPawn : public AGhostCameraPawn
{
	typedef AGhostCameraPawn Super;
public:
	APlayerPawn();

	virtual void BeginPlay() override;

	virtual void SetupPlayerInput() override;

	virtual void Tick(float Delta) override;

private:
	AMissileSwarmSystem* mMissileSwarmSystem;

	void TriggerMissileSwarm(float Delta);

	void FireArcProjectile(float Delta);

	const float mArcProjectileDelay = 0.1f;

	float mArcProjectileCoolTime = 0;

	bool mbMissileAiming = false;

	void Interaction(float Delta);

	IInteractionInterface* mInteractionTarget = nullptr;
};