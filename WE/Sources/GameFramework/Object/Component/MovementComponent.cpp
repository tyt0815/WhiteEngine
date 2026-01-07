#include "MovementComponent.h"
#include "GameFramework/Object/Actor/Actor.h"

void WMovementComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);

	XMVECTOR ScaledVelocity = XMLoadFloat3(&mVelocity);
	XMVectorScale(ScaledVelocity, DeltaTime);
	XMFLOAT4X4 World = GetOwner()->GetRootComponent()->GetWorldMatrix();
	XMMATRIX WorldMat = XMLoadFloat4x4(&World);
	XMVECTOR WorldDirectionV = XMVector3TransformNormal(ScaledVelocity, WorldMat);
	XMFLOAT3 WorldDirection;
	XMStoreFloat3(&WorldDirection, WorldDirectionV);
	MoveOwner(WorldDirection);
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
