#pragma once
#include "GameFramework/Object/Actor/Actor.h"
#include "GameFramework/InputSystem/InputSystem.h"

class APawn : public AActor
{
public:
	virtual void SetupPlayerInput() {};

	virtual void Tick_PrePhysics(float Delta) override;

protected:
	void AddMovementInput(const XMFLOAT3& WorldDirection, float ScaleValue);

	void AddYawInput(float Value);

	void AddPitchInput(float Value);

	void AddRollInput(float Value);

	WCameraComponent* mCameraComponent = nullptr;

	float mMoveSpeed = 30.0f;

	float mCameraSpeed = 1.f;

public:
	inline WCameraComponent* GetCameraComponent() const
	{
		return mCameraComponent;
	}
	inline void SetCameraComponent(WCameraComponent* CameraComponent)
	{
		mCameraComponent = CameraComponent;
	}
};