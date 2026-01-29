#pragma once
#include "Actor/Actor.h"

class IHitInterface;

class AHitManager : public AActor
{
public:
	void AddInstigator(AActor* Instigator);

	/**
	 * @brief 관리 대상에서 인스티게이터를 제거합니다.
	 * @param Instigator 제거할 액터.
	 * @param bDestroyIfEmpty true일 경우, 남은 인스티게이터가 없으면 매니저를 즉시 파괴(Decommission)합니다.
	 */
	void RemoveInstigator(AActor* Instigator, bool bDestroyIfEmpty = true);

	// 해당 Instigator가 목록에 존재할 경우 목록의 인덱스 반환
	// 없을 경우 -1 반환
	int GetInstigatorIndex(AActor* Instigator);

	// 해당 HittedActors가 목록에 존재할 경우 목록의 인덱스 반환
	// 없을 경우 -1 반환
	int GetHittedActorIndex(IHitInterface* HittedActors);

	bool DestroyIfEmpty();

	void Hit(IHitInterface* Victim, AActor* Instigator);

private:
	TArray<IHitInterface*> mHittedActors;

	TArray<AActor*> mInstigators;
};