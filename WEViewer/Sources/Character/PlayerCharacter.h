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

    XMFLOAT3 mVelocity = { 0, 0, 0 };
    XMFLOAT3 mInputDirection = { 0, 0, 0 }; // WASD 방향 누적

    bool mbIsGrounded = true;
    const float mMoveSpeed = 10.0f;
    const float mJumpImpulse = 5.0f;
    const float mGravity = -9.8f;
};