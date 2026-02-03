#include "MovementComponent.h"
#include "GameFramework/Object/Actor/Actor.h"

WMovementComponent::WMovementComponent()
{
	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_High);
}

void WMovementComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 프레임마다 Owner를 이동시킴
	if (AActor* Owner = GetOwner().lock().get())
	{
		XMFLOAT4 Quat = Owner->GetActorTransform().GetQuaternionRotationFloat4();

		XMVECTOR V = XMVector3Rotate(XMLoadFloat3(&mVelocity), XMLoadFloat4(&Quat));

		XMFLOAT3 ScaledVelocity;
		XMStoreFloat3(&ScaledVelocity, XMVectorScale(V, DeltaTime));
		MoveOwner(ScaledVelocity);
	}
}


void WMovementComponent::MoveOwner(XMFLOAT3 Velocity)
{
	if (AActor* Owner = GetOwner().lock().get())
	{
		XMFLOAT3 WorldLocation = Owner->GetActorTransform().Translation;
		XMVECTOR DirectionVector = XMLoadFloat3(&Velocity);
		XMVECTOR LocationVector = XMLoadFloat3(&WorldLocation);
		LocationVector = DirectX::XMVectorAdd(LocationVector, DirectionVector);
		XMStoreFloat3(&WorldLocation, LocationVector);
		Owner->SetActorLocation(WorldLocation);
	}
}
