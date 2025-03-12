#pragma once
#include "GameFramework/Object/Actor/Actor.h"

class APawn : public AActor
{
public:
	virtual void SetupPlayerInput();
	virtual void Tick(float Delta) override;

protected:
	void AddMovementInput(const XMFLOAT3& WorldDirection, float ScaleValue);
	void AddYawInput(float Value);
	void AddPitchInput(float Value);
	void AddRollInput(float Value);
};