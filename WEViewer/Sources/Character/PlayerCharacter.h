#pragma once
#include "Character/Character.h"
#include "GameFramework/Interface/HitInterface.h"

class APlayerCharacter : public ACharacter, public IHitInterface
{
    typedef ACharacter Super;
public:
    APlayerCharacter();
    virtual void Tick(float DeltaSecond) override;
    virtual void SetupPlayerInput() override;

    virtual void OnHit(WSceneComponent* Instigator, WPhysicsComponent* HittedComponent, XMFLOAT3 ImpulseDir, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance, float Damage) override;

public:
    void AddForce(const XMFLOAT3& Force);

    void AddImpulse(const XMFLOAT3& Impulse);

private:
    void FireArcProjectile(float Delta);
    const float mArcProjectileDelay = 0.1f;
    float mArcProjectileCoolTime = 0;

    void Interaction(float Delta);
    class IInteractionInterface* mInteractionTarget = nullptr;

    WSceneComponent* mCameraPivot;

    void Look(FMouseInputParameter Parameter);

    // 입력 처리 함수
    void MoveForward(float Delta);
    void MoveBack(float Delta);
    void MoveLeft(float Delta);
    void MoveRight(float Delta);
    void Jump(float Delta);

    bool mbIsGrounded = true;

    // 물리 상태
    XMFLOAT3 mVelocity = { 0, 0, 0 };
    XMVECTOR mPendingForce = XMVectorZero();

    // 설정값
    const float mMass = 1.0f;
    const float mFriction = 8.0f; // 바닥 마찰력 (속도 감쇠용)
    const float mMaxSpeed = 5.0f;      // 수평 이동 제한 속도

    XMFLOAT3 mInputDirection = { 0, 0, 0 }; // WASD 방향 누적

    const float mAcceleration = 50.0f;  // 초당 가속도
    const float mJumpImpulse = 5.0f;
    const float mGravity = -9.8f;
};