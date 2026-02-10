#include "HitReactor.h"
#include "World/World.h"

XMFLOAT4 GetDamageColor(float Damage)
{
    // 구간 설정 (0: White, 100: Red, 200: Green, 300: Blue, 400: Yellow, 500: Cyan, 600: Magenta, 700: Black)
    struct ColorStep { float Val; XMFLOAT4 Color; };
    static ColorStep Steps[] = {
        { 0.f,   {1, 1, 1, 1} }, // White
        { 100.f, {1, 0, 0, 1} }, // Red
        { 200.f, {0, 1, 0, 1} }, // Green
        { 300.f, {0, 0, 1, 1} }, // Blue
        { 400.f, {1, 1, 0, 1} }, // Yellow
        { 500.f, {0, 1, 1, 1} }, // Cyan
        { 600.f, {1, 0, 1, 1} }, // Magenta
        { 700.f, {0, 0, 0, 1} }  // Black
    };

    if (Damage <= 0.f) return Steps[0].Color;
    if (Damage >= 700.f) return Steps[7].Color;

    // 현재 Damage가 속한 구간 찾기
    for (int i = 0; i < 7; ++i)
    {
        if (Damage >= Steps[i].Val && Damage <= Steps[i + 1].Val)
        {
            float Alpha = (Damage - Steps[i].Val) / 100.f; // 0.0 ~ 1.0 사이 보간값

            return XMFLOAT4(
                Steps[i].Color.x + Alpha * (Steps[i + 1].Color.x - Steps[i].Color.x),
                Steps[i].Color.y + Alpha * (Steps[i + 1].Color.y - Steps[i].Color.y),
                Steps[i].Color.z + Alpha * (Steps[i + 1].Color.z - Steps[i].Color.z),
                1.0f
            );
        }
    }
    return Steps[7].Color;
}

void AHitReactor::OnHit(AActor* Instigator, WPhysicsComponent* HittedComp, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Damage)
{
    // 1. 데미지에 따른 색상 결정
    XMFLOAT4 DebugColor = GetDamageColor(Damage);
    float LineLength = 10.0f; // 가시성을 위해 길이를 조금 늘렸습니다.
    float Duration = 2.0f;

    // 2. 피격 지점에서 공격자 방향으로 라인 (파란색 계열 유지 혹은 데미지 색상)
    GetWorld()->DrawDebugLine(ImpactPoint, Instigator->GetActorLocation(), DebugColor, Duration);

    // 3. 충돌 지점(ImpactPoint) 기준 XYZ 축 디버그 라인
    // X축
    GetWorld()->DrawDebugLine(ImpactPoint,
        XMFLOAT3(ImpactPoint.x + LineLength, ImpactPoint.y, ImpactPoint.z), DebugColor, Duration);
    // Y축
    GetWorld()->DrawDebugLine(ImpactPoint,
        XMFLOAT3(ImpactPoint.x, ImpactPoint.y + LineLength, ImpactPoint.z), DebugColor, Duration);
    // Z축
    GetWorld()->DrawDebugLine(ImpactPoint,
        XMFLOAT3(ImpactPoint.x, ImpactPoint.y, ImpactPoint.z + LineLength), DebugColor, Duration);
}