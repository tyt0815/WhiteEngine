#include "MovementComponent.h"
#include "GameFramework/Object/Actor/Actor.h"

void WMovementComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);

	// 프레임마다 Owner를 이동시킴
	XMFLOAT4 Quat = GetOwner()->GetActorTransform().GetQuaternionRotation();
	XMVECTOR V = XMVector3Rotate(XMLoadFloat3(&mVelocity), XMLoadFloat4(&Quat));

	XMFLOAT3 ScaledVelocity;
	XMStoreFloat3(&ScaledVelocity, XMVectorScale(V, DeltaTime));
	MoveOwner(ScaledVelocity);
}


void WMovementComponent::MoveOwner(XMFLOAT3 Velocity)
{
	AActor* Owner = GetOwner();
	XMFLOAT3 WorldLocation = Owner->GetActorTransform().Translation;
	XMVECTOR DirectionVector = XMLoadFloat3(&Velocity);
	XMVECTOR LocationVector = XMLoadFloat3(&WorldLocation);
	LocationVector = DirectX::XMVectorAdd(LocationVector, DirectionVector);
	XMStoreFloat3(&WorldLocation, LocationVector);
	Owner->SetActorLocation(WorldLocation);
}
