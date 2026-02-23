#include "MovementComponent.h"
#include "GameFramework/Object/Actor/Actor.h"

WMovementComponent::WMovementComponent()
{
	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_High);

	RegisterWProperty("Velocity", &mVelocity);
}

void WMovementComponent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    XMVECTOR WorldVelocityV = XMLoadFloat3(&mVelocity);

    XMVECTOR DeltaPosV = XMVectorScale(WorldVelocityV, DeltaTime);

	XMFLOAT3 WorldLocation = GetWorldLocation();
	XMVECTOR LocationVector = XMLoadFloat3(&WorldLocation);
	LocationVector = DirectX::XMVectorAdd(LocationVector, DeltaPosV);
	XMStoreFloat3(&WorldLocation, LocationVector);
	SetWorldLocation(WorldLocation);
}

void WMovementComponent::BeginComponent()
{
	Super::BeginComponent();

	XMFLOAT4 CurrQuat = GetWorldQuatRotation();
	XMVECTOR CurrQuatV = XMLoadFloat4(&CurrQuat);
	XMVECTOR LocalVelocityV = XMLoadFloat3(&mInitialVelocity);

	XMVECTOR WorldVelocityV = XMVector3Rotate(LocalVelocityV, CurrQuatV);
	XMStoreFloat3(&mVelocity, WorldVelocityV);
}
