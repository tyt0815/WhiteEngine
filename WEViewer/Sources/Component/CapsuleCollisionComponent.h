#pragma once
#include "PrimitiveCollisionComponent.h"

class WCapsuleCollisionComponent : public WPrimitiveCollisionComponent
{
    typedef WPrimitiveCollisionComponent Super;

public:
    WCapsuleCollisionComponent();

    // 추상 함수 구현: 실제 캡슐 스윕 트레이스 수행
    virtual void TraceShape(XMFLOAT3 Start, FTransform CurrTransform, const TArray<AActor*>& ActorsToIgnore, FHitResult& Hit) override;

    // 캡슐 크기 설정
    void SetCapsuleSize(float InRadius, float InHalfHeight) { mRadius = InRadius; mHalfHeight = InHalfHeight; }

protected:
    float mRadius = 1.0f;
    float mHalfHeight = 1.0f;
};

REGISTER_COMPONENT(WCapsuleCollisionComponent);