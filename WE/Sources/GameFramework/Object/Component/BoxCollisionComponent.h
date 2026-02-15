#pragma once
#include "PrimitiveCollisionComponent.h"

class WBoxCollisionComponent : public WPrimitiveCollisionComponent
{
    typedef WPrimitiveCollisionComponent Super;

public:
    WBoxCollisionComponent();

    // 추상 함수 구현: 실제 박스 스윕 트레이스 수행
    virtual void TraceShape(XMFLOAT3 Start, FTransform CurrTransform, const TArray<AActor*>& ActorsToIgnore, FHitResult& Hit) override;

    // 박스 크기 설정 (Center로부터 각 축의 반 너비)
    void SetExtent(const XMFLOAT3& InExtent) { mExtent = InExtent; }
    const XMFLOAT3& GetExtent() const { return mExtent; }

protected:
    XMFLOAT3 mExtent = { 1.0f, 1.0f, 1.0f };
};

REGISTER_COMPONENT(WBoxCollisionComponent);