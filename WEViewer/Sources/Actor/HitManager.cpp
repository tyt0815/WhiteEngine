#include "HitManager.h"
#include "Interface/HitInterface.h"

void AHitManager::AddInstigator(AActor* Instigator)
{
	if (GetInstigatorIndex(Instigator) == -1)
	{
		mInstigators.push_back(Instigator);
	}
}

void AHitManager::RemoveInstigator(AActor* Instigator, bool bDestroyIfEmpty)
{
	int i = GetInstigatorIndex(Instigator);
	if (i >= 0)
	{
		mInstigators.erase(mInstigators.begin() + i);
	}

	if (bDestroyIfEmpty)
	{
		DestroyIfEmpty();
	}
}

int AHitManager::GetInstigatorIndex(AActor* Instigator)
{
	auto Iter = std::find(mInstigators.begin(), mInstigators.end(), Instigator);
	
	return Iter == mInstigators.end() ? -1 : (int)(Iter - mInstigators.begin());
}

int AHitManager::GetHittedActorIndex(IHitInterface* HittedActors)
{
	auto Iter = std::find(mHittedActors.begin(), mHittedActors.end(), HittedActors);

	return Iter == mHittedActors.end() ? -1 : (int)(Iter - mHittedActors.begin());
}

bool AHitManager::DestroyIfEmpty()
{
	if (mInstigators.empty())
	{
		Destroy();
		return true;
	}
	return false;
}

void AHitManager::Hit(IHitInterface* Victim, AActor* Instigator)
{
	if (GetHittedActorIndex(Victim) == -1)
	{
		mHittedActors.push_back(Victim);
		Victim->OnHit(Instigator);
	}
}
