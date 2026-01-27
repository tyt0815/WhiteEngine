#include "ProjectileMovementComponent.h"
#include "GameFramework/Object/Actor/Actor.h"

void WProjectileMovementComponent::TickComponent(float DeltaTime)
{
    mLifeTimeElapsed += DeltaTime;
    auto Owner = GetOwner().lock();
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
                    // 2. 턴 리밋 적용 (mHomingTurnLimit가 도 단위라고 가정 시)
                    float MaxStep = XMConvertToRadians(mHomingTurnLimit);

                    // 핵심: Slerp처럼 비율 계산
                    float Alpha = (mHomingTurnLimit <= 0) ? 1.0f : min(1.0f, MaxStep / Radian * DeltaTime);
                    // 3. 축 계산 및 쿼터니언 생성
                    XMVECTOR AxisV = XMVector3Normalize(XMVector3Cross(ForwardV, ToTargetUnitV));
                    if (XMVector3Equal(AxisV, XMVectorZero()))
                    {
                        XMFLOAT3 Right = Owner->GetRightVector();
                        XMVECTOR RightV = XMLoadFloat3(&Right);
                        AxisV = XMVector3Normalize(XMVector3Cross(RightV, ToTargetUnitV));
                    }
                    XMVECTOR RotationQuatV = XMQuaternionRotationAxis(AxisV, Radian * Alpha);

                    // 4. 새로운 방향 벡터 계산
                    XMVECTOR NewForwardV = XMVector3Rotate(ForwardV, RotationQuatV);
                    XMFLOAT3 Up = Owner->GetUpVector();
                    XMVECTOR NewUpV = XMVector3Rotate(XMLoadFloat3(&Up), RotationQuatV);
                    XMFLOAT3 Right = Owner->GetRightVector();
                    XMVECTOR NewRightV = XMVector3Rotate(XMLoadFloat3(&Right), RotationQuatV);
                    
                    XMFLOAT3 NewRotation = FDXMath::GetEulerRotationFromVectors(NewForwardV, NewRightV, NewUpV);
                    Owner->SetActorRotation(NewRotation);
                }
            }
        }
    }

    Super::TickComponent(DeltaTime);
}

