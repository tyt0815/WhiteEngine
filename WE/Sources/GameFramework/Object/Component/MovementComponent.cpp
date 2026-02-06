#include "MovementComponent.h"
#include "GameFramework/Object/Actor/Actor.h"

WMovementComponent::WMovementComponent()
{
	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_High);
}

void WMovementComponent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (AActor* Owner = GetOwner().lock().get())
    {
        XMVECTOR WorldVelocityV = XMLoadFloat3(&mVelocity);

        XMVECTOR DeltaPosV = XMVectorScale(WorldVelocityV, DeltaTime);

        XMFLOAT3 ScaledVelocity;
        XMStoreFloat3(&ScaledVelocity, DeltaPosV);

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
