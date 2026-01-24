
#pragma once
#include "Actor/Actor.h"
#include "ColdLaunchAnimPlayer.h"

class AMissileSwarmSystem : public AActor
{
public:
	template<typename TColdLaunchAnimPlayer, typename TProjectile>
	void Fire(int Row, int Col);
	
private:

};

template<typename TColdLaunchAnimPlayer, typename TProjectile>
inline void AMissileSwarmSystem::Fire(int Row, int Col)
{
	if (Row == 0 || Col == 0)
	{
		return;
	}
	XMFLOAT3 Origin = GetActorLocation();
	XMVECTOR O = XMLoadFloat3(&Origin);
	XMFLOAT3 Forward = GetFowardVector();
	XMFLOAT3 Right = GetRightVector();
	XMVECTOR F = XMLoadFloat3(&Forward);
	XMVECTOR R = XMLoadFloat3(&Right);

	float Gap = 1;	// 간격

	float halfRow = (Row - 1) * 0.5f;
	float halfCol = (Col - 1) * 0.5f;

	for (int r = 0; r < Row; ++r)
	{
		float rowPos = (float)r - halfRow;
		for (int c = 0; c < Col; ++c)
		{
			if (TSharedPtr<AColdLaunchAnimPlayer> AnimPlayer = GetWorld()->SpawnActor<TColdLaunchAnimPlayer>().lock())
			{
				// 2. 현재 인덱스에서 절반 값을 빼서 중앙 상대 좌표 구하기
				// r=0일 때 -halfRow (뒤쪽), r=Row-1일 때 +halfRow (앞쪽)
				float colPos = (float)c - halfCol;

				// 3. 실제 월드 좌표 계산
				// Origin(O)에서 Forward(F)로 rowPos만큼, Right(R)로 colPos만큼 이동
				XMVECTOR targetPosV = O + (F * rowPos * Gap) + (R * colPos * Gap);

				// 4. 결과값 저장
				XMFLOAT3 finalPos;
				XMStoreFloat3(&finalPos, targetPosV);

				AnimPlayer->SetActorLocation(finalPos);
				AnimPlayer->PlayAnim<TProjectile>();
			}
		}
	}
}
