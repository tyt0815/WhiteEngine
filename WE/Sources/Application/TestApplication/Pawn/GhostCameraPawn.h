#pragma once
#include "GameFramework/Object/Pawn/Pawn.h"

class WCameraComponent;

class AGhostCameraPawn : public APawn
{
public:
	AGhostCameraPawn();

private:
	WCameraComponent* Camera;
};