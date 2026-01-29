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

    // 1. 루프 밖에서 한 번만 로드 (기억하고 있습니다! 변수에 담아서 주소 전달)
    XMFLOAT3 Forward = GetForwardVector();
    XMFLOAT3 Up = GetUpVector();
    XMFLOAT3 Right = GetRightVector();
    XMFLOAT3 ActorLoc = GetActorLocation();

    XMVECTOR ForwardV = XMLoadFloat3(&Forward);
    XMVECTOR RightV = XMLoadFloat3(&Right);
    XMVECTOR UpV = XMLoadFloat3(&Up);

    // 월드 행렬 로드
    XMMATRIX W = GetRootComponent().lock()->GetWorldMatrix();

    XMFLOAT4 Red = { 1, 0, 0, 1 };

    // 2. X자 방향 벡터들도 루프 밖에서 미리 계산 (성능 최적화)
    XMVECTOR XDirs[4] = {
        XMVector3Normalize(ForwardV + RightV),
        XMVector3Normalize(ForwardV - RightV),
        XMVector3Normalize(-ForwardV + RightV),
        XMVector3Normalize(-ForwardV - RightV)
    };

    for (const auto& Row : mTargetPoints)
    {
        for (const auto& Loc : Row)
        {
            // 로컬 포인트를 월드로 변환
            XMVECTOR OriginV = XMVector3Transform(XMLoadFloat3(&Loc), W);
            OriginV += UpV * 0.1f; // 0.01은 너무 낮아 바닥에 묻힐 수 있음

            XMFLOAT3 StartPos;
            XMStoreFloat3(&StartPos, OriginV);

            // 3. 위쪽 라인 (시작점을 StartPos로 변경!)
            XMFLOAT3 UpEnd;
            XMStoreFloat3(&UpEnd, OriginV + (UpV * 3.0f));
            World->DrawDebugLine(StartPos, UpEnd, Red, 0);

            // 4. 바닥 X자 라인
            for (int i = 0; i < 4; ++i)
            {
                XMFLOAT3 End;
                XMStoreFloat3(&End, OriginV + (XDirs[i] * 0.5f));
                World->DrawDebugLine(StartPos, End, Red, 0);
            }
        }
    }
}

void ATargetMarker::CreateTargetMarkers(int Row, int Col, float GridInterval)
{
    mTargetPoints.clear();
    mTargetPoints.resize(Row); // Row가 전방 방향(Forward) 개수가 됨

    // 깨끗한 단위 벡터 정의
    XMVECTOR LocalForwardV = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    XMVECTOR LocalRightV = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

    // 1. 전방 루프 (Row): 액터 기준 앞으로 전진
    for (int r = 0; r < Row; ++r)
    {
        mTargetPoints[r].resize(Col); // 각 줄마다 좌우(Col) 개수 할당

        // 전방 거리 계산 (앞으로 멀어질수록 r이 커짐)
        float ForwardOffset = r * GridInterval;

        // 2. 좌우 루프 (Col): 중심 기준 좌우로 퍼짐
        for (int c = 0; c < Col; ++c)
        {
            // 중앙 정렬을 위해 좌우 Offset 계산
            float RightOffset = (c - (Col - 1) * 0.5f) * GridInterval;

            // 로컬 위치 계산 (y값인 Up은 변하지 않음)
            XMVECTOR RelativePosV = (LocalForwardV * ForwardOffset) + (LocalRightV * RightOffset);

            XMFLOAT3 RelativePos;
            XMStoreFloat3(&RelativePos, RelativePosV);

            mTargetPoints[r][c] = RelativePos;
        }
    }
}

