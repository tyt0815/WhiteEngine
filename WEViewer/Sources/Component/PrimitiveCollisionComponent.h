#pragma once
#include "Component/SceneComponent.h"
#include "Interface/CollisionGenerator.h"
#include "Physics/HitResult.h"

class WPrimitiveCollisionComponent : public WSceneComponent, public FCollisionGeneratorBase
{
    typedef WSceneComponent Super;

public:
    WPrimitiveCollisionComponent();

    virtual void Tick(float DeltaSecond) override final;

    virtual void BeginComponent() override;

    // 인터페이스 구현: 단일 충돌체이므로 사실상 초기화 역할
    virtual void GenerateCollision() override;


protected:
    virtual void OnActivate() override;

    virtual void OnDeactivate() override;

    virtual void ActivateCollision() override;

    virtual void DeactivateCollision() override;


protected:
    virtual void TraceShape(XMFLOAT3 Start, FTransform CurrTransform, const TArray<AActor*>& ActorsToIgnore, FHitResult& Hit) {};

    XMFLOAT3 mPrevLocation;

    float mElapsedTime = 0.0f;

    bool mbGenerateCollision = false;

public:
};

REGISTER_COMPONENT(WPrimitiveCollisionComponent);