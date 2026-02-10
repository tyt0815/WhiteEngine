#include "PrimitiveCollisionComponent.h"
#include "Actor/Actor.h"
#include "World/World.h"
#include "Physics/HitResult.h"

WPrimitiveCollisionComponent::WPrimitiveCollisionComponent()
{
    SetTickGroup(ETickGroup::ETG_PostPhysics, ETickPriority::ETP_High);
}

void WPrimitiveCollisionComponent::BeginComponent()
{
    Super::BeginComponent();
}

void WPrimitiveCollisionComponent::GenerateCollision()
{
    mbGenerateCollision = true;
    mPrevLocation = GetWorldLocation();
}

void WPrimitiveCollisionComponent::OnActivate()
{
    Super::OnActivate();
    ActivateCollision();
}

void WPrimitiveCollisionComponent::OnDeactivate()
{
    Super::OnDeactivate();
    DeactivateCollision();
}

void WPrimitiveCollisionComponent::ActivateCollision()
{
    mPrevLocation = GetWorldLocation();
}

void WPrimitiveCollisionComponent::DeactivateCollision()
{
}

void WPrimitiveCollisionComponent::Tick(float DeltaSecond)
{
    Super::Tick(DeltaSecond);

    if (!mbGenerateCollision)
    {
        return;
    }

    constexpr float FIXED_DELTA = 1.0f / 60.0f;
    mElapsedTime += DeltaSecond;
    if (mElapsedTime < FIXED_DELTA) return;
    mElapsedTime = 0;

    FTransform CurrTransform = GetWorldTransform();
    FHitResult Hit;
    TArray<AActor*> ActorsToIgnore;

    if (auto Owner = GetOwner().lock())
    {
        ActorsToIgnore.push_back(Owner.get());
    }

    TraceShape(mPrevLocation, CurrTransform, ActorsToIgnore, Hit);

    if (auto HittedActor = Hit.Actor.lock())
    {
        mOnCollision.Broadcast(
            HittedActor.get(),
            Hit.HitComponent.lock().get(),
            Hit.ImpactPoint,
            Hit.Normal,
            Hit.Distance
        );
    }

    mPrevLocation = CurrTransform.Translation;
}