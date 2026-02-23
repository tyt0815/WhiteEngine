#include "HitReactorManager.h"
#include "Actor/HitReactor.h"


AHitReactorManager::AHitReactorManager()
{
}

void AHitReactorManager::Reset()
{
	DestroyHitReactors();
	SpawnHitReactors();
}

void AHitReactorManager::DestroyHitReactors()
{
	for (AHitReactor* Actor : mHitReactors)
	{
		Actor->Destroy();
	}

	mHitReactors.clear();
}
