#include "MissileGridManager.h"
#include "Interface/HitInterface.h"
#include "TopAttackMissile.h"

void AMissileGridManager::AddMissile(ATopAttackMissile* Instigator)
{
	if (GetMissileIndex(Instigator) == -1)
	{
		mMissiles.push_back(Instigator);
	}

	const auto& Deque = Instigator->mHomingPathMarkerDeque;
	for (auto Iter = Deque.begin(); Iter != Deque.end(); ++Iter)
	{
		mHomingPaths.push_back(Iter->lock().get());
	}
	
}

void AMissileGridManager::RemoveMissile(ATopAttackMissile* Instigator, bool bSmartDestroy)
{
	int i = GetMissileIndex(Instigator);
	if (i >= 0)
	{
		--mMissileCounting;
		mMissiles.erase(mMissiles.begin() + i);

		if (bSmartDestroy)
		{
			SmartDestroy();
		}
	}	
}

int AMissileGridManager::GetMissileIndex(ATopAttackMissile* Instigator)
{
	auto Iter = std::find(mMissiles.begin(), mMissiles.end(), Instigator);
	
	return Iter == mMissiles.end() ? -1 : (int)(Iter - mMissiles.begin());
}

int AMissileGridManager::GetHittedActorIndex(IHitInterface* HittedActors)
{
	auto Iter = std::find(mHittedActors.begin(), mHittedActors.end(), HittedActors);

	return Iter == mHittedActors.end() ? -1 : (int)(Iter - mHittedActors.begin());
}

bool AMissileGridManager::SmartDestroy()
{
	if (mMissileCounting <= 0)
	{
		for (AActor* Path : mHomingPaths)
		{
			Path->Destroy();
		}

		Destroy();
		return true;
	}
	return false;
}

void AMissileGridManager::Hit(IHitInterface* Victim, ATopAttackMissile* Instigator)
{
	if (GetHittedActorIndex(Victim) == -1)
	{
		mHittedActors.push_back(Victim);
		Victim->OnHit(Instigator);
	}
}
