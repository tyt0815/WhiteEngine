#pragma once
#include "GameFramework/Object/Pawn/Pawn.h"

class WCameraComponent;

class AGhostCameraPawn : public APawn
{
public:
	AGhostCameraPawn();
	virtual void SetupPlayerInput() override;

private:
	void MoveForward(float Delta);
	void MoveBackward(float Delta);
	void MoveRight(float Delta);
	void MoveLeft(float Delta);
	void MoveUp(float Delta);
	void MoveDown(float Delta);
	void Look(FMouseInputParameter Parameter);
	void SpeedUp(float Delta);
	void SpeedDown(float Delta);

	float mMoveSpeedScaler = 1.0f;
	virtual void LeftClick(FMouseInputParameter Parameter) {};
};