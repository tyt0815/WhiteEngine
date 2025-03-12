#include "Pawn.h"
#include "Utility/Timer.h"

void APawn::Tick(float Delta)
{
}

void APawn::AddMovementInput(const XMFLOAT3& WorldDirection, float ScaleValue)
{
	ScaleValue *= GetAppTimer()->GetDeltaTime() * mMoveSpeed;
	XMFLOAT3 WorldLocation = GetTransform().Translation;
	XMVECTOR DirectionVector = XMLoadFloat3(&WorldDirection);
	DirectionVector = DirectX::XMVectorScale(DirectionVector, ScaleValue);
	XMVECTOR LocationVector = XMLoadFloat3(&WorldLocation);
	LocationVector = DirectX::XMVectorAdd(LocationVector, DirectionVector);
	XMStoreFloat3(&WorldLocation, LocationVector);
	SetLocation(WorldLocation);
}

void APawn::AddYawInput(float Value)
{
	Value *= GetAppTimer()->GetDeltaTime() * mCameraSpeed;
	XMFLOAT3 Rotation = GetTransform().Rotation;
	Rotation.y += Value;
	SetRotation(Rotation);
}

void APawn::AddPitchInput(float Value)
{
	Value *= GetAppTimer()->GetDeltaTime() * mCameraSpeed;
	XMFLOAT3 Rotation = GetTransform().Rotation;
	Rotation.x += Value;
	SetRotation(Rotation);
}

void APawn::AddRollInput(float Value)
{
	Value *= GetAppTimer()->GetDeltaTime() * mCameraSpeed;
	XMFLOAT3 Rotation = GetTransform().Rotation;
	Rotation.z += Value;
	SetRotation(Rotation);
}
