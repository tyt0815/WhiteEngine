#include "BoxCollisionComponent.h"
#include "World/World.h"
#include "Actor/Actor.h"

WBoxCollisionComponent::WBoxCollisionComponent()
{
}

void WBoxCollisionComponent::TraceShape(XMFLOAT3 Start, FTransform CurrTransform, const TArray<AActor*>& ActorsToIgnore, FHitResult& Hit)
{    
    XMVECTOR ExtentV = XMLoadFloat3(&mExtent);
    XMVECTOR ScaleV = XMLoadFloat3(&CurrTransform.Scale);
    XMFLOAT3 Extent;
    XMStoreFloat3(&Extent, XMVectorMultiply(ExtentV, ScaleV));

    GetWorld()->BoxTrace(
        Start,
        CurrTransform.Translation,
        Extent,
        GetWorldRotation(),
        ActorsToIgnore,
        Hit,
        mbDebug,           // DrawDebug (필요 시 변수화)
        mCollisionInterval    // LifeTime
    );
}