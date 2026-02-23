#pragma once
#include "Actor/Actor.h"

class AHitReactor;

class AHitReactorManager : public AActor
{
	typedef AActor Super;
public:
	AHitReactorManager();

public:
	void Reset();

protected:
	virtual void SpawnHitReactors() = 0;
	void DestroyHitReactors();

	TArray<AHitReactor*> mHitReactors;
private:

};