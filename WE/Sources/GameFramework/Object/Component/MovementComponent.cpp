#include "MovementComponent.h"
#include "GameFramework/Object/Actor/Actor.h"

void WMovementComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);

	XMVECTOR ScaledVelocityV = XMLoadFloat3(&mVelocity);
	XMVectorScale(ScaledVelocityV, DeltaTime);
	XMFLOAT3 ScaledVelocity;
	XMStoreFloat3(&ScaledVelocity, ScaledVelocityV);
	MoveOwner(ScaledVelocity);
}

void WMovementComponent::MoveOwner(XMFLOAT3 WorldDirection)
{
	AActor* Owner = GetOwner();
	XMFLOAT3 WorldLocation = Owner->GetActorTransform().Translation;
	XMVECTOR DirectionVector = XMLoadFloat3(&WorldDirection);
	XMVECTOR LocationVector = XMLoadFloat3(&WorldLocation);
	LocationVector = DirectX::XMVectorAdd(LocationVector, DirectionVector);
	XMStoreFloat3(&WorldLocation, LocationVector);
	Owner->SetActorLocation(WorldLocation);
}
