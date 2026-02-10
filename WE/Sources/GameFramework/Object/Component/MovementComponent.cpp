#include "MovementComponent.h"
#include "GameFramework/Object/Actor/Actor.h"

WMovementComponent::WMovementComponent()
{
	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_High);
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