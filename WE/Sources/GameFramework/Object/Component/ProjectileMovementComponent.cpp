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
                XMFLOAT3 Forward = Owner->GetFowardVector();
                XMVECTOR F = XMLoadFloat3(&Forward);
                XMVECTOR ToTargetUnit = XMVector3Normalize(ToTargetV);

                // 1. 두 벡터 사이의 각도 계산
                float Radian = XMVectorGetX(XMVector3AngleBetweenNormals(F, ToTargetUnit));
                float Degree = 180.0f * Radian / FDXMath::Pi;

                if (Radian > 0.001f) // 각도 차이가 있을 때만 회전
                {
                    // 2. 턴 리밋 적용 (mHomingTurnLimit가 도 단위라고 가정 시)
                    float MaxStep = XMConvertToRadians(mHomingTurnLimit) * DeltaTime;

                    // 핵심: Slerp처럼 비율 계산
                    float Alpha = (mHomingTurnLimit <= 0) ? 1.0f : min(1.0f, MaxStep / Radian);
                    Alpha = 1;

                    // 3. 축 계산 및 쿼터니언 생성
                    XMVECTOR Axis = XMVector3Normalize(XMVector3Cross(F, ToTargetUnit));
                    XMVECTOR RotationQuat = XMQuaternionRotationAxis(Axis, Radian * Alpha);

                    // 4. 새로운 방향 벡터 계산
                    XMVECTOR NewForward = XMVector3Rotate(F, RotationQuat);

                    // 5. 회전 적용 (LookTo 방식보다 쿼터니언 직접 생성이 더 안전)
                    // 현재 액터의 쿼터니언에 델타 회전(RotationQuat)을 곱합니다.
                    XMVECTOR CurrentQuat = FDXMath::EulerToQuaternionVector(Owner->GetActorRotation());
                    XMFLOAT4 a;
                    XMStoreFloat4(&a, CurrentQuat);
                    auto b = FDXMath::QuaternionToEuler(a);
                    XMVECTOR FinalQuat = XMQuaternionMultiply(CurrentQuat, RotationQuat);

                    XMFLOAT4 FinalQuatStr;
                    XMStoreFloat4(&FinalQuatStr, FinalQuat);
                    Owner->SetActorRotation(FDXMath::QuaternionToEuler(FinalQuatStr));
                }
            }
        }
    }

    Super::TickComponent(DeltaTime);
}

