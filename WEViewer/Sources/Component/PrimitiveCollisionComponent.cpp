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
    mPrevLocation = GetWorldLocation();
}

void WPrimitiveCollisionComponent::OnDeactivate()
{
    Super::OnDeactivate();
}

void WPrimitiveCollisionComponent::Tick(float DeltaSecond)
{
    Super::Tick(DeltaSecond);
    if (!mbGenerateCollision) return;

    UpdateActorsToIgnore(DeltaSecond);

    mElapsedTime += DeltaSecond;
    if (mElapsedTime < mCollisionInterval) return;
    mElapsedTime = 0;

    FTransform CurrTransform = GetWorldTransform();
    FHitResult Hit;

    TArray<AActor*> TraceIgnore = mCachedIgnoreList;
    if (auto Owner = GetOwner().lock()) TraceIgnore.push_back(Owner.get());

    // 하위 클래스에서 구현된 Trace 실행
    TraceShape(mPrevLocation, CurrTransform, TraceIgnore, Hit);

    if (!Hit.Actor.expired())
    {
        ProcessHit(Hit);
    }

    mPrevLocation = CurrTransform.Translation;
}