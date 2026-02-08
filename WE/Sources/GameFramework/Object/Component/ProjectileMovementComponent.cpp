#include "ProjectileMovementComponent.h"
#include "GameFramework/Object/Actor/Actor.h"

WProjectileMovementComponent::WProjectileMovementComponent()
{
}

void WProjectileMovementComponent::Tick(float DeltaTime)
{
    mLifeTimeElapsed += DeltaTime;
    AActor* Owner = GetOwner().lock().get();

    if (!Owner || (mLifeSpan > 0 && mLifeTimeElapsed > mLifeSpan))
    {
        if (Owner) Owner->Destroy();
        return;
    }

    // 1. 중력 적용
    constexpr float GravityConstant = 9.8f;
    XMVECTOR CurrentVelocityV = XMLoadFloat3(&mVelocity);
    XMVECTOR GravityV = XMVectorSet(0.0f, -GravityConstant * mGravityScale * DeltaTime, 0.0f, 0.0f);
    CurrentVelocityV = XMVectorAdd(CurrentVelocityV, GravityV);
    XMVECTOR RotationQuatV = XMQuaternionIdentity();
    bool bCalcRotQuatV = true;

    // 1-1. 외부 가속도 적용 (AddForce로 누적된 값)
    XMVECTOR ExternalAccelV = XMLoadFloat3(&mExternalAcceleration);
    CurrentVelocityV = XMVectorAdd(CurrentVelocityV, XMVectorScale(ExternalAccelV, DeltaTime));

    // 2. 호밍 로직 (속도 벡터의 방향을 꺾음)
    if (mbHomingProjectile)
    {
        if (TSharedPtr<WSceneComponent> Target = mHomingTarget.lock())
        {
            XMFLOAT3 CurrLoc = Owner->GetActorLocation();
            XMVECTOR CurrLocV = XMLoadFloat3(&CurrLoc);
            XMFLOAT3 TargetLoc = Target->GetWorldLocation();
            XMVECTOR TargetLocV = XMLoadFloat3(&TargetLoc);
            XMVECTOR ToTargetV = XMVectorSubtract(TargetLocV, CurrLocV);

            if (XMVectorGetX(XMVector3LengthSq(ToTargetV)) > 0.00001f)
            {
                XMVECTOR CurrentDirV = XMVector3Normalize(CurrentVelocityV);
                XMVECTOR ToTargetUnitV = XMVector3Normalize(ToTargetV);

                // 속도 방향과 타겟 방향 사이의 각도 계산
                float Radian = XMVectorGetX(XMVector3AngleBetweenNormals(CurrentDirV, ToTargetUnitV));
                if (Radian > 0.0001f)
                {
                    float TurnSpeedRad = XMConvertToRadians(mHomingTurnLimit);
                    float MaxAngleThisFrame = (mHomingTurnLimit > 0) ? TurnSpeedRad * DeltaTime : Radian;
                    float ActualRotation = min(Radian, MaxAngleThisFrame);

                    XMVECTOR AxisV = XMVector3Normalize(XMVector3Cross(CurrentDirV, ToTargetUnitV));
                    if (XMVector3Equal(AxisV, XMVectorZero()))
                    {
                        XMFLOAT3 Right = Owner->GetRightVector();
                        XMVECTOR RightV = XMLoadFloat3(&Right);
                        AxisV = XMVector3Normalize(XMVector3Cross(RightV, ToTargetUnitV));
                    }

                    RotationQuatV = XMQuaternionRotationAxis(AxisV, ActualRotation);
                    bCalcRotQuatV = false;
                    // 속도 벡터 자체를 회전시킴
                    CurrentVelocityV = XMVector3Rotate(CurrentVelocityV, RotationQuatV);
                }
            }
        }
    }

    if (bCalcRotQuatV)
    {
        XMVECTOR DirV = XMVector3Normalize(CurrentVelocityV);
        XMFLOAT3 Forward = Owner->GetForwardVector();
        XMVECTOR ForwardV = XMLoadFloat3(&Forward);

        float Radian = XMVectorGetX(XMVector3AngleBetweenNormals(ForwardV, DirV));
        if (Radian > 0.0001f)
        {
            XMVECTOR AxisV = XMVector3Normalize(XMVector3Cross(ForwardV, DirV));
            if (XMVector3Equal(AxisV, XMVectorZero()))
            {
                XMFLOAT3 Up = Owner->GetUpVector();
                AxisV = XMLoadFloat3(&Up);
            }
            // 현재 방향과 속도 방향 사이의 차이만큼 회전 쿼터니언 생성
            RotationQuatV = XMQuaternionRotationAxis(AxisV, Radian);
        }
    }

    XMVECTOR ForwardV = XMVector3Normalize(CurrentVelocityV);
    // 3. 전방 추진 가속도 적용 (스칼라)
    if (mAcceleration != 0.0f)
    {
        // 호밍/중력이 적용된 후의 '현재 진행 방향'으로 가속    
        CurrentVelocityV = XMVectorAdd(CurrentVelocityV, XMVectorScale(ForwardV, mAcceleration * DeltaTime));
    }

    // 4. 속도 제한 (Max Speed)
    if (mMaxSpeed > 0.0f)
    {
        float SpeedSq = XMVectorGetX(XMVector3LengthSq(CurrentVelocityV));
        if (SpeedSq > mMaxSpeed * mMaxSpeed)
        {
            CurrentVelocityV = XMVectorScale(ForwardV, mMaxSpeed);
        }
    }

    // 5. 최종 속도 저장
    XMStoreFloat3(&mVelocity, CurrentVelocityV);

    // 6. 비주얼 정렬 (투사체가 실제 이동 방향을 바라보게 함)
    if (XMVectorGetX(XMVector3LengthSq(CurrentVelocityV)) > 0.01f)
    {
        XMFLOAT4 CurrQuat = Owner->GetActorQuaternion();
        XMVECTOR CurrQuatV = XMLoadFloat4(&CurrQuat);
        
        XMFLOAT4 FinalQuat;
        XMStoreFloat4(&FinalQuat, XMQuaternionMultiply(CurrQuatV, RotationQuatV));
        XMFLOAT3 FinalRotation = FDXMath::QuaternionToEuler(FinalQuat);

        Owner->SetActorRotation(FinalRotation);
    }

    mExternalAcceleration = { 0.f, 0.f, 0.f };

    Super::Tick(DeltaTime);
}

void WProjectileMovementComponent::BeginComponent()
{
    Super::BeginComponent();

    TSharedPtr<AActor> Owner = GetOwner().lock();
    assert(Owner);

    // 1. 로컬 초기 속도를 월드 속도로 변환
    // mInitialVelocity는 로컬 기준(예: {500, 0, 0})이라고 가정합니다.
    XMFLOAT4 CurrQuat = Owner->GetActorQuaternion();
    XMVECTOR CurrQuatV = XMLoadFloat4(&CurrQuat);
    XMVECTOR LocalVelocityV = XMLoadFloat3(&mInitialVelocity);

    // 액터의 회전(Quat)을 이용하여 로컬 벡터를 월드 방향으로 회전시킵니다.
    XMVECTOR WorldVelocityV = XMVector3Rotate(LocalVelocityV, CurrQuatV);

    // 2. 부모 MovementComponent의 mVelocity(월드 속도)에 저장
    XMStoreFloat3(&mVelocity, WorldVelocityV);

    // 4. 액터가 실제 날아가는 방향(월드 속도 방향)을 바라보게 회전
    if (XMVectorGetX(XMVector3Length(WorldVelocityV)) > 0.0001f)
    {
        XMVECTOR DirV = XMVector3Normalize(WorldVelocityV);

        // 현재 액터의 전방 벡터
        XMFLOAT3 Forward = Owner->GetForwardVector();
        XMVECTOR ForwardV = XMLoadFloat3(&Forward);

        // 두 벡터 사이의 각도(Radian) 구하기
        float Radian = XMVectorGetX(XMVector3AngleBetweenNormals(ForwardV, DirV));

        if (Radian > 0.0001f)
        {
            // 회전 축 구하기 (외적)
            XMVECTOR AxisV = XMVector3Normalize(XMVector3Cross(ForwardV, DirV));

            // 만약 두 벡터가 180도 반대라 외적 축이 0이 나온다면 임의의 상방 축 사용
            if (XMVector3Equal(AxisV, XMVectorZero()))
            {
                XMFLOAT3 Right = Owner->GetRightVector();
                XMVECTOR RightV = XMLoadFloat3(&Right);
                AxisV = XMVector3Normalize(XMVector3Cross(RightV, DirV));
            }

            // 추가될 회전 쿼터니언 생성
            XMVECTOR DeltaQuatV = XMQuaternionRotationAxis(AxisV, Radian);

            // 기존 쿼터니언에 곱하여 최종 회전 결정 (순서: 기존 * 추가)
            XMVECTOR FinalQuatV = XMQuaternionMultiply(CurrQuatV, DeltaQuatV);

            // 4. 액터에 최종 회전 적용 (오일러 변환 후 적용)
            XMFLOAT4 FinalQuat;
            XMStoreFloat4(&FinalQuat, FinalQuatV);
            XMFLOAT3 FinalRotation = FDXMath::QuaternionToEuler(FinalQuat);

            Owner->SetActorRotation(FinalRotation);
        }
    }
}

void WProjectileMovementComponent::SetHomingTarget(WSceneComponent* Target)
{
    if (Target)
    {
        mHomingTarget = Target->GetWeakPtr<WSceneComponent>();
    }
    else
    {
        mHomingTarget.reset();
    }
}

void WProjectileMovementComponent::AddForce(const XMFLOAT3& Force)
{
    XMVECTOR CurrentAccel = XMLoadFloat3(&mExternalAcceleration);
    XMVECTOR NewForce = XMLoadFloat3(&Force);
    XMStoreFloat3(&mExternalAcceleration, XMVectorAdd(CurrentAccel, NewForce));
}
