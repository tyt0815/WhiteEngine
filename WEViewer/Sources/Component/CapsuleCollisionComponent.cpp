#include "CapsuleCollisionComponent.h"
#include "World/World.h"
#include "Actor/Actor.h"

WCapsuleCollisionComponent::WCapsuleCollisionComponent()
{
}

void WCapsuleCollisionComponent::TraceShape(XMFLOAT3 Start, FTransform CurrTransform, const TArray<AActor*>& ActorsToIgnore, FHitResult& Hit)
{
    float ScaleMax = FDXMath::Max(CurrTransform.Scale.x, CurrTransform.Scale.z);
    float FinalRadius = mRadius * ScaleMax;
    float FinalHalfHeight = mHalfHeight * CurrTransform.Scale.y;

    GetWorld()->CapsuleTrace(
        Start,
        CurrTransform.Translation,
        FinalRadius,
        FinalHalfHeight,
        GetWorldRotation(),
        ActorsToIgnore,
        Hit,
        mbDebug,           // DrawDebug
        1.0f / 60.0f    // LifeTime
    );
}