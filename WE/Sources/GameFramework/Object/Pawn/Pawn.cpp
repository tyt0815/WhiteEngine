#include "Pawn.h"

void APawn::SetupPlayerInput()
{
	// TODO
}

void APawn::Tick(float Delta)
{
}

void APawn::AddMovementInput(const XMFLOAT3& WorldDirection, float ScaleValue)
{
	// TODO
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
	XMFLOAT3 Rotation = GetTransform().Rotation;
	Rotation.y += Value;
	SetRotation(Rotation);
}

void APawn::AddPitchInput(float Value)
{
	XMFLOAT3 Rotation = GetTransform().Rotation;
	Rotation.x += Value;
	SetRotation(Rotation);
}

void APawn::AddRollInput(float Value)
{
	XMFLOAT3 Rotation = GetTransform().Rotation;
	Rotation.z += Value;
	SetRotation(Rotation);
}
