#pragma once
#include "Pawn/GhostCameraPawn.h"
#include "Actor/MissileSwarmSystem.h"

class APlayerPawn : public AGhostCameraPawn
{
	typedef AGhostCameraPawn Super;
public:
	APlayerPawn();

	virtual void BeginPlay() override;

	virtual void SetupPlayerInput() override;

	virtual void Tick(float Delta) override;

private:
	TWeakPtr<AMissileSwarmSystem> mMissileSwarmSystem;

	void MissileSwarm(float Delta);

	bool mbMissileAiming = false;
};