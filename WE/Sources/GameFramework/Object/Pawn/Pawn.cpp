#include "Pawn.h"
#include "Utility/Timer.h"

void APawn::Tick_PrePhysics(float Delta)
{
}

void APawn::AddMovementInput(const XMFLOAT3& WorldDirection, float ScaleValue)
{
	ScaleValue *= (float)GetEngineTimer()->GetDeltaTime() * mMoveSpeed;
	XMFLOAT3 WorldLocation = GetActorTransform().Translation;
	XMVECTOR DirectionVector = XMLoadFloat3(&WorldDirection);
	DirectionVector = DirectX::XMVectorScale(DirectionVector, ScaleValue);
	XMVECTOR LocationVector = XMLoadFloat3(&WorldLocation);
	LocationVector = DirectX::XMVectorAdd(LocationVector, DirectionVector);
	XMStoreFloat3(&WorldLocation, LocationVector);
	SetActorLocation(WorldLocation);
}

void APawn::AddYawInput(float Value)
{
	Value *= mCameraSpeed * 0.2f;
	XMFLOAT3 Rotation = GetActorTransform().Rotation;
	Rotation.y += Value;
	SetActorRotation(Rotation);
}

void APawn::AddPitchInput(float Value)
{
	Value *= mCameraSpeed * 0.2f;
	XMFLOAT3 Rotation = GetActorTransform().Rotation;
	Rotation.x = FDXMath::Clamp(Rotation.x + Value, -89.0f, 89.0f);
	SetActorRotation(Rotation);
}

void APawn::AddRollInput(float Value)
{
	Value *= mCameraSpeed * 0.2f;
	XMFLOAT3 Rotation = GetActorTransform().Rotation;
	Rotation.z += Value;
	SetActorRotation(Rotation);
}
