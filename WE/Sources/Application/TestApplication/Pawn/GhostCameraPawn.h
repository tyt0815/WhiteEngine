#pragma once
#include "GameFramework/Object/Pawn/Pawn.h"

class WCameraComponent;

class AGhostCameraPawn : public APawn
{
public:
	AGhostCameraPawn();
	virtual void SetupPlayerInput() override;

private:
	void MoveForward();
	void MoveBackward();
	void MoveRight();
	void MoveLeft();
	void MoveUp();
	void MoveDown();
	void Look(FMouseInputParameter Parameter);
	WCameraComponent* Camera;
};