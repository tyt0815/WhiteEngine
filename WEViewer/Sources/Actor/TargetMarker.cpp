#include "TargetMarker.h"
#include "World/World.h"

ATargetMarker::ATargetMarker()
{
	SetTickGroup(ETickGroup::ETG_PostPhysics, ETickPriority::ETP_Low);
}

void ATargetMarker::Tick(float DeltaSecond)
{
    Super::Tick(DeltaSecond);

    WWorld* World = GetWorld();

    // 1. 필요한 벡터 및 위치 로드 (변수 거쳐서 주소 전달 규칙 준수)
    XMFLOAT3 Forward = GetForwardVector();
    XMFLOAT3 Up = GetUpVector();
    XMFLOAT3 Right = GetRightVector();
    XMFLOAT3 Origin = GetActorLocation();

    XMVECTOR ForwardV = XMLoadFloat3(&Forward);
    XMVECTOR RightV = XMLoadFloat3(&Right);
    XMVECTOR UpV = XMLoadFloat3(&Up);
    XMVECTOR OriginV = XMLoadFloat3(&Origin);

    // 지면 묻힘 방지를 위해 살짝 띄움
    XMVECTOR BaseV = OriginV + (UpV * 0.1f);

    XMFLOAT4 Red = { 1, 0, 0, 1 };
    XMFLOAT3 StartPos;
    XMStoreFloat3(&StartPos, BaseV);

    // 2. 위쪽 라인 (길이 3)
    XMFLOAT3 UpEnd;
    XMStoreFloat3(&UpEnd, BaseV + (UpV * 3.0f));
    World->DrawDebugLine(StartPos, UpEnd, Red, 0);

    // 3. 바닥 X자 라인 (길이 0.5)
    // 대각선 4방향 벡터 계산
    XMVECTOR XDirs[4] = {
        XMVector3Normalize(ForwardV + RightV),
        XMVector3Normalize(ForwardV - RightV),
        XMVector3Normalize(-ForwardV + RightV),
        XMVector3Normalize(-ForwardV - RightV)
    };

    for (int i = 0; i < 4; ++i)
    {
        XMFLOAT3 End;
        XMStoreFloat3(&End, BaseV + (XDirs[i] * 0.5f));
        World->DrawDebugLine(StartPos, End, Red, 0);
    }
}

