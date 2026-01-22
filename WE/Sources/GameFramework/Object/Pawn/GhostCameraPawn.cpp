#include "GhostCameraPawn.h"
#include "GameFramework/Object/World/World.h"
#include "GameFramework/Object/Component/CameraComponent.h"

AGhostCameraPawn::AGhostCameraPawn()
{
	mCameraComponent = CreateComponent<WCameraComponent>();
	SetRootComponent(mCameraComponent);
}

void AGhostCameraPawn::SetupPlayerInput()
{
	// TODO
	GetInputSystemManager()->BindKeyboardAction('w', this, &AGhostCameraPawn::MoveForward);	
	GetInputSystemManager()->BindKeyboardAction('s', this, &AGhostCameraPawn::MoveBackward);
	GetInputSystemManager()->BindKeyboardAction('d', this, &AGhostCameraPawn::MoveRight);
	GetInputSystemManager()->BindKeyboardAction('a', this, &AGhostCameraPawn::MoveLeft);
	GetInputSystemManager()->BindKeyboardAction('e', this, &AGhostCameraPawn::MoveUp);
	GetInputSystemManager()->BindKeyboardAction('q', this, &AGhostCameraPawn::MoveDown);
	GetInputSystemManager()->BindMouseAction(EMIT_Move, this, &AGhostCameraPawn::Look);
	GetInputSystemManager()->BindMouseAction(EMIT_LDown, this, &AGhostCameraPawn::LeftClick);
}

void AGhostCameraPawn::MoveForward(float Delta)
{

	AddMovementInput(GetFowardVector(), Delta);
}

void AGhostCameraPawn::MoveBackward(float Delta)
{
	AddMovementInput(GetFowardVector(), -Delta);
}

void AGhostCameraPawn::MoveRight(float Delta)
{
	AddMovementInput(GetRightVector(), Delta);
}

void AGhostCameraPawn::MoveLeft(float Delta)
{
	AddMovementInput(GetRightVector(), -Delta);
}

void AGhostCameraPawn::MoveUp(float Delta)
{
	AddMovementInput({ 0.0f, 1.0f, 0.0f }, Delta);
}

void AGhostCameraPawn::MoveDown(float Delta)
{
	AddMovementInput({ 0.0f, -1.0f, 0.0f }, Delta);
}

void AGhostCameraPawn::Look(FMouseInputParameter Parameter)
{
	AddYawInput(static_cast<float>(Parameter.X));
	AddPitchInput(static_cast<float>(Parameter.Y));
}