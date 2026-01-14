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

void AGhostCameraPawn::MoveForward()
{

	AddMovementInput(GetFowardVector(), 1.0f);
}

void AGhostCameraPawn::MoveBackward()
{
	AddMovementInput(GetFowardVector(), -1.0f);
}

void AGhostCameraPawn::MoveRight()
{
	AddMovementInput(GetRightVector(), 1.0f);
}

void AGhostCameraPawn::MoveLeft()
{
	AddMovementInput(GetRightVector(), -1.0f);
}

void AGhostCameraPawn::MoveUp()
{
	AddMovementInput({ 0.0f, 1.0f, 0.0f }, 1.0f);
}

void AGhostCameraPawn::MoveDown()
{
	AddMovementInput({ 0.0f, -1.0f, 0.0f }, 1.0f);
}

void AGhostCameraPawn::Look(FMouseInputParameter Parameter)
{
	AddYawInput(static_cast<float>(Parameter.X));
	AddPitchInput(static_cast<float>(Parameter.Y));
}