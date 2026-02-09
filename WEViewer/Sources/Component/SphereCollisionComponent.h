#pragma once
#include "PrimitiveCollisionComponent.h"

class WSphereCollisionComponent : public WPrimitiveCollisionComponent
{
    typedef WPrimitiveCollisionComponent Super;

public:
    WSphereCollisionComponent();

    // 추상 함수 구현: 실제 구체 스윕 트레이스 수행
    virtual void TraceShape(XMFLOAT3 Start, FTransform CurrTransform, const TArray<AActor*>& ActorsToIgnore, FHitResult& Hit) override;

    // 구체 반지름 설정
    void SetRadius(float InRadius) { mRadius = InRadius; }
    float GetRadius() const { return mRadius; }

protected:
    // 단일 반지름 값 사용
    float mRadius = 1.0f;
};

REGISTER_COMPONENT(WSphereCollisionComponent);