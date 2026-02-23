#include "GhostCameraPawn.h"
#include "GameFramework/Object/World/World.h"
#include "GameFramework/Object/Component/CameraComponent.h"

AGhostCameraPawn::AGhostCameraPawn()
{
	mCameraComponent = CreateComponent<WCameraComponent>()->GetWeakPtr<WCameraComponent>();
	SetRootComponent(mCameraComponent);
}

void AGhostCameraPawn::SetupPlayerInput()
{
	// TODO
	GetInputSystemManager()->BindKeyboardAction('w', EKeyboardInputType::EKIT_Down, this, &AGhostCameraPawn::MoveForward);	
	GetInputSystemManager()->BindKeyboardAction('s', EKeyboardInputType::EKIT_Down, this, &AGhostCameraPawn::MoveBackward);
	GetInputSystemManager()->BindKeyboardAction('d', EKeyboardInputType::EKIT_Down, this, &AGhostCameraPawn::MoveRight);
	GetInputSystemManager()->BindKeyboardAction('a', EKeyboardInputType::EKIT_Down, this, &AGhostCameraPawn::MoveLeft);
	GetInputSystemManager()->BindKeyboardAction('e', EKeyboardInputType::EKIT_Down, this, &AGhostCameraPawn::MoveUp);
	GetInputSystemManager()->BindKeyboardAction('q', EKeyboardInputType::EKIT_Down, this, &AGhostCameraPawn::MoveDown);
	GetInputSystemManager()->BindKeyboardAction('z', EKeyboardInputType::EKIT_Down, this, &AGhostCameraPawn::SpeedUp);
	GetInputSystemManager()->BindKeyboardAction('x', EKeyboardInputType::EKIT_Down, this, &AGhostCameraPawn::SpeedDown);
	GetInputSystemManager()->BindMouseAction(EMIT_Move, this, &AGhostCameraPawn::Look);
	GetInputSystemManager()->BindMouseAction(EMIT_LDown, this, &AGhostCameraPawn::LeftClick);
}

void AGhostCameraPawn::MoveForward(float Delta)
{

	AddMovementInput(GetForwardVector(), Delta * mMoveSpeedScaler);
}

void AGhostCameraPawn::MoveBackward(float Delta)
{
	AddMovementInput(GetForwardVector(), -Delta * mMoveSpeedScaler);
}

void AGhostCameraPawn::MoveRight(float Delta)
{
	AddMovementInput(GetRightVector(), Delta * mMoveSpeedScaler);
}

void AGhostCameraPawn::MoveLeft(float Delta)
{
	AddMovementInput(GetRightVector(), -Delta * mMoveSpeedScaler);
}

void AGhostCameraPawn::MoveUp(float Delta)
{
	AddMovementInput({ 0.0f, 1.0f, 0.0f }, Delta * mMoveSpeedScaler);
}

void AGhostCameraPawn::MoveDown(float Delta)
{
	AddMovementInput({ 0.0f, -1.0f, 0.0f }, Delta * mMoveSpeedScaler);
}

void AGhostCameraPawn::Look(FMouseInputParameter Parameter)
{
	AddYawInput(static_cast<float>(Parameter.X));
	AddPitchInput(static_cast<float>(Parameter.Y));
}

void AGhostCameraPawn::SpeedUp(float Delta)
{
	mMoveSpeedScaler += 1.0f * Delta;
}

void AGhostCameraPawn::SpeedDown(float Delta)
{
	mMoveSpeedScaler = max(0.5f, mMoveSpeedScaler - 1.0f * Delta);
}
