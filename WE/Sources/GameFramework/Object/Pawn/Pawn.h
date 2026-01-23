#pragma once
#include "GameFramework/Object/Actor/Actor.h"
#include "GameFramework/InputSystem/InputSystem.h"

class APawn : public AActor
{
public:
	virtual void SetupPlayerInput() {};

protected:
	void AddMovementInput(const XMFLOAT3& WorldDirection, float ScaleValue);

	void AddYawInput(float Value);

	void AddPitchInput(float Value);

	void AddRollInput(float Value);

	TWeakPtr<WCameraComponent> mCameraComponent;

	float mMoveSpeed = 30.0f;

	float mCameraSpeed = 1.f;

public:
	inline TWeakPtr<WCameraComponent> GetCameraComponent() const
	{
		return mCameraComponent;
	}
	inline void SetCameraComponent(const TWeakPtr<WCameraComponent>& CameraComponent)
	{
		mCameraComponent = CameraComponent;
	}
};