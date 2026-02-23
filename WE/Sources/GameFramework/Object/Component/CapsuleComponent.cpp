#include "CapsuleComponent.h"
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>

JPH::ShapeRefC WCapsuleComponent::CreatePhysicsShape()
{
    // 스케일이 적용된 최종 수치 계산
    float FinalRadius = GetScaledRadius();
    float FinalHalfHeight = GetScaledHalfHeight();

    // Jolt 캡슐 생성 (반지름과 절반 높이 전달)
    JPH::CapsuleShapeSettings CapsuleSettings(FinalHalfHeight, FinalRadius);
    CapsuleSettings.SetEmbedded();

    JPH::ShapeSettings::ShapeResult ShapeResult = CapsuleSettings.Create();
    return ShapeResult.Get();
}

float WCapsuleComponent::GetScaledRadius()
{
    XMFLOAT3 Scale = GetWorldTransform().Scale;
    // 캡슐의 수평 단면은 원이므로 X와 Z 스케일 중 더 큰 값을 기준으로 반지름 결정
    float MaxHorizontalScale = FDXMath::Max(Scale.x, Scale.z);
    return mRadius * MaxHorizontalScale;
}

float WCapsuleComponent::GetScaledHalfHeight()
{
    XMFLOAT3 Scale = GetWorldTransform().Scale;
    // Y축(언리얼/Jolt 표준 높이축) 스케일을 적용
    return mHalfHeight * Scale.y;
}