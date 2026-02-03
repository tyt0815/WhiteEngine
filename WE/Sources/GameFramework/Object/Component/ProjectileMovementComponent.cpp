#include "ProjectileMovementComponent.h"
#include "GameFramework/Object/Actor/Actor.h"

WProjectileMovementComponent::WProjectileMovementComponent()
{
    RegisterWProperty("LifeSpan", &mLifeSpan);
    RegisterWProperty("HomingTurnLimit", &mHomingTurnLimit);
    RegisterWProperty("Homing", &mbHomingProjectile);
    RegisterWProperty("Speed", &mSpeed);
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

    if (mbHomingProjectile)
    {
        if (TSharedPtr<WSceneComponent> Target = mHomingTarget.lock())
        {
            XMFLOAT3 CurrentLocation = Owner->GetActorLocation();
            XMVECTOR CurrLocV = XMLoadFloat3(&CurrentLocation);
            XMFLOAT3 TargetLocation = Target->GetWorldLocation();
            XMVECTOR TargetLocV = XMLoadFloat3(&TargetLocation);
            XMVECTOR ToTargetV = XMVectorSubtract(TargetLocV, CurrLocV);

            float DistanceSq = XMVectorGetX(XMVector3LengthSq(ToTargetV));
            if (DistanceSq > 0.00001f) // 거리 체크
            {
                XMFLOAT3 Forward = Owner->GetForwardVector();
                XMVECTOR ForwardV = XMLoadFloat3(&Forward);
                XMVECTOR ToTargetUnitV = XMVector3Normalize(ToTargetV);

                // 1. 두 벡터 사이의 각도 계산
                float Radian = XMVectorGetX(XMVector3AngleBetweenNormals(ForwardV, ToTargetUnitV));
                if (Radian > 0.0001f) // 각도 차이가 있을 때만 회전
                {
                    // mHomingTurnLimit를 "초당 회전 각도(Degree/sec)"라고 정의합시다.
                    float TurnSpeedRad = XMConvertToRadians(mHomingTurnLimit);

                    // 1. 이번 프레임에 최대로 회전할 수 있는 '각도 크기'를 구합니다.
                    float MaxAngleThisFrame = TurnSpeedRad * DeltaTime;

                    // 2. 가야 할 각도(Radian)와 회전 가능 각도(MaxAngleThisFrame) 중 작은 것을 선택합니다.
                    // 만약 남은 각도가 이번 프레임 회전량보다 작으면 그냥 남은 각도만큼만 돕니다.
                    float ActualRotation = min(Radian, MaxAngleThisFrame);

                    // 3. 축 계산 및 쿼터니언 생성
                    XMVECTOR AxisV = XMVector3Normalize(XMVector3Cross(ForwardV, ToTargetUnitV));
                    if (XMVector3Equal(AxisV, XMVectorZero()))
                    {
                        XMFLOAT3 Right = Owner->GetRightVector();
                        XMVECTOR RightV = XMLoadFloat3(&Right);
                        AxisV = XMVector3Normalize(XMVector3Cross(RightV, ToTargetUnitV));
                    }
                    XMVECTOR RotationQuatV = XMQuaternionRotationAxis(AxisV, ActualRotation);

                    // 4. 새로운 방향 벡터 계산                    

                    XMFLOAT4 CurrQuat = GetOwner().lock()->GetActorQuaternion();
                    XMVECTOR CurrQuatV = XMLoadFloat4(&CurrQuat);

                    XMFLOAT4 NewQuat;
                    XMStoreFloat4(&NewQuat, XMQuaternionMultiply(CurrQuatV, RotationQuatV));
                    XMFLOAT3 NewRotation = FDXMath::QuaternionToEuler(NewQuat);

                    Owner->SetActorRotation(NewRotation);
                }
            }
        }
    }

    Super::Tick(DeltaTime);
}

void WProjectileMovementComponent::BeginComponent()
{
    Super::BeginComponent();

    SetSpeed(mSpeed);
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

void WProjectileMovementComponent::SetSpeed(float Value)
{
    mSpeed = Value;
    mVelocity = XMFLOAT3(0, 0, mSpeed);
}