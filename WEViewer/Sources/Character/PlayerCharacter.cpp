#include "PlayerCharacter.h"
#include "Component/CameraComponent.h"
#include "Component/CapsuleComponent.h"
#include "Component/StaticMeshComponent.h"
#include "World/World.h"
#include "Interface/InteractionInterface.h"

APlayerCharacter::APlayerCharacter()
{
	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_Low);

    WCapsuleComponent* Capsule = GetCapsule();
    Capsule->SetHalfHeight(0.5f);
    Capsule->SetRadius(0.4f);
    Capsule->mFriction = 1;
     //Capsule->mMaxLinearVelocity = 1;
    Capsule->mGravityFactor = 0;

	mCameraPivot = CreateComponent<WSceneComponent>();
	mCameraPivot->SetupAttachment(GetRootComponent());
    mCameraPivot->SetRelativeLocation(XMFLOAT3(0, 0.7f, 0));
	mCameraComponent = CreateComponent<WCameraComponent>();
	mCameraComponent->SetupAttachment(mCameraPivot);
	mCameraComponent->SetRelativeLocation(XMFLOAT3(0, 0, 0));

    WStaticMeshComponent* SMComp = CreateComponent<WStaticMeshComponent>();
    SMComp->SetupAttachment(GetRootComponent());
    SMComp->SetStaticMesh("SM_MetalCylinder");
}

void APlayerCharacter::Tick(float DeltaSecond)
{
    Super::Tick(DeltaSecond);

    WCapsuleComponent* Capsule = GetCapsule();
    const float HalfHeight = Capsule->GetScaledHalfHeight();
    const float HalfRadius = Capsule->GetScaledRadius();

    XMFLOAT3 CurrLoc = GetActorLocation();
    XMVECTOR vCurrLoc = XMLoadFloat3(&CurrLoc);

    if (!mbIsGrounded)
    {
        AddForce(XMFLOAT3(0, mGravity * mMass, 0));
    }

    // WASD 이동 의지를 Force로 변환
    XMVECTOR vInput = XMLoadFloat3(&mInputDirection);
    if (!XMVector3Equal(vInput, XMVectorZero()))
    {
        vInput = XMVector3Normalize(vInput);
        float Yaw = XMConvertToRadians(mCameraPivot->GetWorldRotation().y);
        XMVECTOR vQuat = XMQuaternionRotationRollPitchYaw(0, Yaw, 0);
        XMVECTOR vDir = XMVector3Rotate(vInput, vQuat) * mAcceleration * mMass;
        if (!mbIsGrounded)
        {
            vDir *= mAirControl;
        }

        // 입력에 따른 추진력 추가
        AddForce(XMFLOAT3(
            XMVectorGetX(vDir),
            0,
            XMVectorGetZ(vDir)
        ));
    }

    // --- 2. 속도 업데이트 (v = v + a*dt) ---
    XMVECTOR vVel = XMLoadFloat3(&mVelocity);

    // a = F / m
    XMVECTOR vAccel = XMVectorScale(mPendingForce, 1.0f / mMass);
    vVel = XMVectorAdd(vVel, XMVectorScale(vAccel, DeltaSecond));

    // --- 3. 저항(마찰력) 처리 ---
    XMVECTOR vHorizontalVel = XMVectorSetY(vVel, 0.0f);
    float VerticalVel = XMVectorGetY(vVel);
    VerticalVel = mbIsGrounded && VerticalVel < 0 ? 0 : VerticalVel;

    // 수평 속도 감쇠 (v = v * (1 - friction * dt))
    float Friction = mbIsGrounded ? mGroundFriction : mAirFriction;
    float Drag = 1.0f - (Friction * DeltaSecond);
    if (Drag < 0) Drag = 0;
    vHorizontalVel = XMVectorScale(vHorizontalVel, Drag);

    // --- 4. 최대 속도 제한 (Clamping) ---
    // 입력에 의한 속도만 제한하고 싶다면 로직을 분리할 수 있지만, 
    // 기본적으로 수평 속도가 mMaxSpeed를 넘지 않게 조절합니다.
    float SpeedSq = XMVectorGetX(XMVector3LengthSq(vHorizontalVel));
    if (SpeedSq > mMaxSpeed * mMaxSpeed)
    {
        vHorizontalVel = XMVectorScale(XMVector3Normalize(vHorizontalVel), mMaxSpeed);
    }

    // 최종 속도 재조합
    vVel = XMVectorSetY(vHorizontalVel, VerticalVel);
    XMStoreFloat3(&mVelocity, vVel);

    // --- 5. 위치 업데이트 ---
    XMVECTOR vNewLoc = XMVectorAdd(vCurrLoc, XMVectorScale(vVel, DeltaSecond));

    XMFLOAT3 FinalLoc;
    XMStoreFloat3(&FinalLoc, vNewLoc);
    SetActorLocation(FinalLoc);
    {
        WCapsuleComponent* Capsule = GetCapsule();
        const XMFLOAT3& TraceStart = FinalLoc;
        XMFLOAT3 TraceEnd = TraceStart;
        TraceEnd.y -= Capsule->GetScaledHalfHeight();

        FHitResult Hit;
        TArray<AActor*> ActorsToIgnore;
        ActorsToIgnore.push_back(this);
        GetWorld()->SphereTrace(TraceStart, TraceEnd, Capsule->GetScaledRadius(), ActorsToIgnore, Hit, true, 0);

        // OnAir
        if (Hit.Actor.expired())
        {
            mbIsGrounded = false;
        }
        else
        {
            if (!mbIsGrounded)
            {
                // FinalLoc.y = Hit.ImpactPoint.y + HalfHeight;
                mVelocity.y = 0;
                mbIsGrounded = true;
            }
        }
    }

    

    // 입력 누적 초기화
    mPendingForce = XMVectorZero();
    mInputDirection = { 0, 0, 0 };

    mArcProjectileCoolTime = max(mArcProjectileCoolTime - DeltaSecond, 0);

    // Interaction
    {
        XMFLOAT3 TraceStart = mCameraComponent->GetWorldLocation();
        XMVECTOR vTraceStart = XMLoadFloat3(&TraceStart);
        XMFLOAT3 Forward = mCameraComponent->GetWorldForwardVector();
        XMVECTOR vForward = XMLoadFloat3(&Forward);
        XMFLOAT3 TraceEnd;
        XMStoreFloat3(&TraceEnd, XMVectorAdd(vTraceStart, XMVectorScale(vForward, 2.0f)));

        TArray<AActor*> ActorsToIgnore;
        ActorsToIgnore.push_back(this);
        FHitResult HitResult;
        GetWorld()->LineTrace(
            TraceStart, TraceEnd,
            ActorsToIgnore,
            HitResult,
            true,
            0
        );

        auto HittedActor = HitResult.Actor.lock();
        IInteractionInterface* Target = dynamic_cast<IInteractionInterface*>(HittedActor.get());

        if (mInteractionTarget != Target)
        {
            if (mInteractionTarget)
            {
                mInteractionTarget->OnEndInteractionFocus();
            }

            if (Target)
            {
                Target->OnBeginInteractionFocus();
            }
            mInteractionTarget = Target;
        }
    }

    vVel = XMLoadFloat3(&mVelocity);
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    WCapsuleComponent* Capsule = GetCapsule();
    Capsule->mOnHitDelegate.Bind(this, &APlayerCharacter::OnCapsuleHit);
    Capsule->mOnExitHitDelegate.Bind(this, &APlayerCharacter::OnExitCapsuleHit);
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

    GetInputSystemManager()->BindKeyboardAction(VK_LBUTTON, EKeyboardInputType::EKIT_Down, this, &APlayerCharacter::FireArcProjectile);
    GetInputSystemManager()->BindKeyboardAction('f', EKeyboardInputType::EKIT_Pressed, this, &APlayerCharacter::Interaction);
}

void APlayerCharacter::OnHit(
    WSceneComponent* Instigator, WPhysicsComponent* HittedComponent,
    XMFLOAT3 ImpulseDir, XMFLOAT3 ImpactPoint,
    XMFLOAT3 Normal, float Distance, float Damage
)
{
    XMVECTOR vImpulse = XMVectorScale(XMLoadFloat3(&ImpulseDir), Damage);

    XMFLOAT3 FinalImpulse;
    XMStoreFloat3(&FinalImpulse, vImpulse);

    AddImpulse(FinalImpulse);
}

void APlayerCharacter::AddForce(const XMFLOAT3& Force)
{
    XMVECTOR vForce = XMLoadFloat3(&Force);
    mPendingForce = XMVectorAdd(mPendingForce, vForce);
}

void APlayerCharacter::AddImpulse(const XMFLOAT3& Impulse)
{
    // I = m * deltaV  =>  deltaV = I / m
    XMVECTOR vImpulse = XMLoadFloat3(&Impulse);
    XMVECTOR vDeltaVel = XMVectorScale(vImpulse, 1.0f / mMass);

    XMVECTOR vCurrentVel = XMLoadFloat3(&mVelocity);
    XMStoreFloat3(&mVelocity, XMVectorAdd(vCurrentVel, vDeltaVel));
}

void APlayerCharacter::OnCapsuleHit(WPhysicsComponent* OtherComp, XMFLOAT3 ImpactPoint)
{
    AActor* Actor = OtherComp->GetOwner<AActor>();

    int Count = (int)std::count_if(mContactedActor.begin(), mContactedActor.end(), [Actor](auto&& OtherWeak) -> bool
        {
            if (TSharedPtr<AActor> Other = OtherWeak.lock())
            {
                return Other.get() == Actor;
            }
            return false;
        });

    assert(Count < 2);

    if (Count == 0)
    {
        mContactedActor.push_back(Actor->GetWeakPtr<AActor>());
    }
}

void APlayerCharacter::OnExitCapsuleHit(WPhysicsComponent* OtherComp)
{
    AActor* Actor = OtherComp->GetOwner<AActor>();

    auto Iter = std::remove_if(mContactedActor.begin(), mContactedActor.end(), [Actor](auto&& OtherActorWeak) {
        auto OtherActor = OtherActorWeak.lock();
        return !OtherActor || OtherActor.get() == Actor; // 만료된 포인터도 이때 같이 청소
        });
    mContactedActor.erase(Iter, mContactedActor.end()); // 실제로 벡터에서 제거
}

void APlayerCharacter::FireArcProjectile(float Delta)
{
    if (mArcProjectileCoolTime > 0)
    {
        return;
    }
    mArcProjectileCoolTime = mArcProjectileDelay;

    FActorSpawnParameter Param;
    Param.Transform = mCameraComponent->GetWorldTransform();
    Param.Transform.Scale = XMFLOAT3(1, 1, 1);

    GetWorld()->SpawnActorByFactory<AActor>("BP_ArcProjectile", Param);
}

void APlayerCharacter::Interaction(float Delta)
{
    if (mInteractionTarget)
    {
        mInteractionTarget->Interaction();
    }
    else
    {
        std::cout << "No interaction target" << std::endl;
    }
}

void APlayerCharacter::MoveForward(float Delta) 
{ 
    mInputDirection.z += 1;
}

void APlayerCharacter::MoveBack(float Delta) 
{ 
    mInputDirection.z -= 1;
}

void APlayerCharacter::MoveRight(float Delta) 
{ 
    mInputDirection.x += 1;
}

void APlayerCharacter::MoveLeft(float Delta) 
{
    mInputDirection.x -= 1;
}

void APlayerCharacter::Jump(float Delta)
{
    if (mbIsGrounded)
    {
        // 점프는 위 방향으로 즉각적인 속도 변화를 주는 Impulse가 적합합니다.
        // v = mJumpImpulse가 되게 하려면, I = m * mJumpImpulse
        AddImpulse(XMFLOAT3(0, mJumpImpulse * mMass, 0));
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
