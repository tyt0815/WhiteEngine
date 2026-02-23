#include "DenseBoxHitReactorManager.h"
#include "BoxHitReactor.h"
#include "World/World.h"

void ADenseBoxHitReactorManager::SpawnHitReactors()
{
	const int X = mSize.x;
	const int Y = mSize.y;
	const int Z = mSize.z;

	WWorld* World = GetWorld();

    // 박스 크기
    const float BoxSize = 1.0f;

    // 부모(생성기)의 현재 월드 위치 가져오기
    FTransform ParentTransform = GetActorTransform();
    const XMFLOAT3& ParentPos = ParentTransform.Translation;
    const XMFLOAT3& ParentScale = ParentTransform.Scale;

    for (int i = 0; i < X; ++i) // 가로
    {
        for (int j = 0; j < Y; ++j) // 높이
        {
            for (int k = 0; k < Z; ++k) // 깊이
            {
                FActorSpawnParameter Param;
                // 부모의 트랜스폼을 기본값으로 복사 (Scale 등 포함)
                Param.Transform = GetActorTransform();

                // 위치 계산: 부모 위치 + (인덱스 * 박스 크기)
                Param.Transform.Translation.x = ParentPos.x + (i * BoxSize * ParentScale.x);
                Param.Transform.Translation.y = ParentPos.y + (j * BoxSize * ParentScale.y);
                Param.Transform.Translation.z = ParentPos.z + (k * BoxSize * ParentScale.z);

                // 스폰
                AHitReactor* Actor = World->SpawnActor<ABoxHitReactor>(Param);
                mHitReactors.push_back(Actor);
            }
        }
    }
}
