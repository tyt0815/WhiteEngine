#include "PlayerCharacter.h"
#include "Component/CameraComponent.h"
#include "Component/CapsuleComponent.h"
#include "Component/StaticMeshComponent.h"
#include "World/World.h"

APlayerCharacter::APlayerCharacter()
{
	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_Low);

	mCameraPivot = CreateComponent<WSceneComponent>();
	mCameraPivot->SetupAttachment(GetRootComponent());
	mCameraComponent = CreateComponent<WCameraComponent>();
	mCameraComponent->SetupAttachment(mCameraPivot);
	mCameraComponent->SetRelativeLocation(XMFLOAT3(0, 0, -5));

    WStaticMeshComponent* SMComp = CreateComponent<WStaticMeshComponent>();
    SMComp->SetupAttachment(GetRootComponent());
    SMComp->SetStaticMesh("SM_MetalRing");
}

void APlayerCharacter::Tick(float DeltaSecond)
{
    Super::Tick(DeltaSecond);

    // --- 1. 수평 속도 계산 (WASD) ---
    XMVECTOR vInput = XMLoadFloat3(&mInputDirection);
    XMVECTOR vDesiredHorizontalVel = XMVectorZero();

    if (!XMVector3Equal(vInput, XMVectorZero()))
    {
        vInput = XMVector3Normalize(vInput);
        float Yaw = XMConvertToRadians(mCameraPivot->GetWorldRotation().y);
        XMVECTOR vQuat = XMQuaternionRotationRollPitchYaw(0, Yaw, 0);

        vDesiredHorizontalVel = XMVector3Rotate(vInput, vQuat);
        vDesiredHorizontalVel = XMVectorScale(vDesiredHorizontalVel, mMoveSpeed);
    }

    // --- 2. 전체 속도 통합 (mVelocity 업데이트) ---
    XMVECTOR vCurrentVel = XMLoadFloat3(&mVelocity);

    // 수평 속도는 매 프레임 입력에 따라 덮어쓰기 (또는 가속도 로직 적용 가능)
    // 수직 속도는 중력에 의해 누적
    float CurrentVerticalVel = XMVectorGetY(vCurrentVel);
    if (!mbIsGrounded)
    {
        CurrentVerticalVel += mGravity * DeltaSecond;
    }

    // 최종 속도 조합 (Horizontal X, Z + Vertical Y)
    XMVECTOR vFinalVel = XMVectorSetY(vDesiredHorizontalVel, CurrentVerticalVel);
    XMStoreFloat3(&mVelocity, vFinalVel);

    // --- 3. 위치 업데이트 ---
    XMFLOAT3 CurrLoc = GetActorLocation();
    XMVECTOR vCurrLoc = XMLoadFloat3(&CurrLoc);

    // 신규 위치 = 현재 위치 + (통합 속도 * 시간)
    XMVECTOR vNewLoc = XMVectorAdd(vCurrLoc, XMVectorScale(vFinalVel, DeltaSecond));

    // --- 4. 바닥 충돌 및 상태 체크 (간이 로직) ---
    XMFLOAT3 FinalLoc;
    XMStoreFloat3(&FinalLoc, vNewLoc);
    {
        WCapsuleComponent* Capsule = GetCapsule();
        float HalfHeight = Capsule->GetScaledHalfHeight() + Capsule->GetScaledRadius();

        const XMFLOAT3& TraceStart = FinalLoc;
        XMFLOAT3 TraceEnd = TraceStart;
        TraceEnd.y -= HalfHeight + 0.01f;
        FHitResult Hit;
        TArray<AActor*> ActorsToIgnore;
        ActorsToIgnore.push_back(this);
        GetWorld()->LineTrace(TraceStart, TraceEnd, ActorsToIgnore, Hit, true, 0);

        // OnAir
        if (Hit.Actor.expired())
        {
            mbIsGrounded = false;
        }
        else
        {
            if (!mbIsGrounded)
            {
                FinalLoc.y = Hit.ImpactPoint.y + HalfHeight;
                mVelocity.y = 0;
                mbIsGrounded = true;
            }
        }
    }

    SetActorLocation(FinalLoc);

    XMFLOAT3 WorldCameraRot = mCameraPivot->GetWorldRotation();
    XMFLOAT3 CharacterRot = XMFLOAT3(0, WorldCameraRot.y, 0);
    SetActorRotation(CharacterRot);
    mCameraPivot->SetWorldRotation(WorldCameraRot);

    // 입력 누적 초기화
    mInputDirection = { 0, 0, 0 };
}

void APlayerCharacter::SetupPlayerInput()
{
    Super::SetupPlayerInput();

    GetInputSystemManager()->BindMouseAction(EMIT_Move, this, &APlayerCharacter::Look);

    // 'EKIT_Down'은 키를 누르고 있는 동안 매 프레임 실행됩니다.
    GetInputSystemManager()->BindKeyboardAction('W', EKeyboardInputType::EKIT_Down, this, &APlayerCharacter::MoveForward);
    GetInputSystemManager()->BindKeyboardAction('S', EKeyboardInputType::EKIT_Down, this, &APlayerCharacter::MoveBack);
    GetInputSystemManager()->BindKeyboardAction('A', EKeyboardInputType::EKIT_Down, this, &APlayerCharacter::MoveLeft);
    GetInputSystemManager()->BindKeyboardAction('D', EKeyboardInputType::EKIT_Down, this, &APlayerCharacter::MoveRight);
    GetInputSystemManager()->BindKeyboardAction(VK_SPACE, EKeyboardInputType::EKIT_Down, this, &APlayerCharacter::Jump);
}

void APlayerCharacter::MoveForward(float Delta) 
{ 
    mInputDirection.z += 1.0f; 
}

void APlayerCharacter::MoveBack(float Delta) 
{ 
    mInputDirection.z -= 1.0f; 
}

void APlayerCharacter::MoveRight(float Delta) 
{ 
    mInputDirection.x += 1.0f; 
}

void APlayerCharacter::MoveLeft(float Delta) 
{
    mInputDirection.x -= 1.0f; 
}

void APlayerCharacter::Jump(float Delta)
{
    if (mbIsGrounded)
    {
        // 수직 속도(y)만 즉시 변경
        mVelocity.y = mJumpImpulse;
        mbIsGrounded = false;
    }
}

void APlayerCharacter::Look(FMouseInputParameter Parameter)
{
    XMFLOAT3 Rot = mCameraPivot->GetRelativeRotation();
    Rot.x = FDXMath::Clamp<float>(Rot.x + Parameter.Y, -89, 89);
    Rot.y += Parameter.X;
    mCameraPivot->SetRelativeRotation(Rot);
}
