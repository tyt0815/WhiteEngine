#pragma once
#include "GameFramework/Object/Actor/Actor.h"
#include "GameFramework/InputSystem/InputSystem.h"

class APawn : public AActor
{
public:
	virtual void SetupPlayerInput() = 0;
	virtual void Tick(float Delta) override;

protected:
	void AddMovementInput(const XMFLOAT3& WorldDirection, float ScaleValue);
	void AddYawInput(float Value);
	void AddPitchInput(float Value);
	void AddRollInput(float Value);

	float mMoveSpeed = 30.0f;
	float mCameraSpeed = 60.f;
};