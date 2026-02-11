#include "SphereCollisionComponent.h"
#include "World/World.h"
#include "Actor/Actor.h"

WSphereCollisionComponent::WSphereCollisionComponent()
{
}

void WSphereCollisionComponent::TraceShape(XMFLOAT3 Start, FTransform CurrTransform, const TArray<AActor*>& ActorsToIgnore, FHitResult& Hit)
{
    // 월드 스케일 중 가장 큰 축을 기준으로 반지름을 계산 (균등 스케일 가정 시 편리)
    float MaxScale = max(CurrTransform.Scale.x, max(CurrTransform.Scale.y, CurrTransform.Scale.z));
    float FinalRadius = mRadius * MaxScale;

    // 아까 만든 회전 인자가 없는 SphereTrace 호출
    GetWorld()->SphereTrace(
        Start,                      // 이전 프레임 위치
        CurrTransform.Translation,   // 현재 프레임 위치
        FinalRadius,                // 스케일이 적용된 반지름
        ActorsToIgnore,
        Hit,
        mbDebug,                       // bDrawDebug
        mCollisionInterval                // DebugDuration
    );
}