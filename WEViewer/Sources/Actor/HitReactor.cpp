#include "HitReactor.h"
#include "World/World.h"

void AHitReactor::OnHit(AActor* Instigator)
{
	GetWorld()->DrawDebugLine(GetActorLocation(), Instigator->GetActorLocation(), XMFLOAT4(0, 0, 1, 1), 2);

    // 2. 추가: XYZ축 디버그 라인 (노란색)
    XMFLOAT3 CurrentPos = GetActorLocation();
    float LineLength = 1.0f; // 축 라인의 길이 (원하는 만큼 조절하세요)
    XMFLOAT4 AxisColor(1, 1, 0, 1); // 노란색 (R, G, B, A)

    // X축 (빨간색 대신 노란색 요청하셨으니 노란색으로 통일)
    GetWorld()->DrawDebugLine(CurrentPos, XMFLOAT3(CurrentPos.x + LineLength, CurrentPos.y, CurrentPos.z), AxisColor, 2);

    // Y축
    GetWorld()->DrawDebugLine(CurrentPos, XMFLOAT3(CurrentPos.x, CurrentPos.y + LineLength, CurrentPos.z), AxisColor, 2);

    // Z축
    GetWorld()->DrawDebugLine(CurrentPos, XMFLOAT3(CurrentPos.x, CurrentPos.y, CurrentPos.z + LineLength), AxisColor, 2);
}
