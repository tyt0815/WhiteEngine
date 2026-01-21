#pragma once

#include "World/DefaultWorld.h"
#include "Actor/Actor.h"
#include <queue>

class WObjectAnimComponent;
class WStaticMeshComponent;
class WBoxComponent;


class AProjectile : public AActor
{
public:
	AProjectile();

private:
	TWeakPtr<WStaticMeshComponent> mStaticMeshComp;

	TWeakPtr<WBoxComponent> mBoxCollision;
};

class AProjectileAnimActor : public AActor
{
	typedef AActor Super;
public:
	AProjectileAnimActor();

	virtual void BeginPlay() override;

	virtual void Tick_PostPhysics(float Delta) override;

protected:
	virtual void OnActivate() override;

	virtual void OnDeactivate() override;

private:
	TWeakPtr<WObjectAnimComponent> mObjectAnimComp;

	TWeakPtr<AProjectile> mProj;

	float mElapsedTime = 0;
};

class AProjSpawner : public AActor
{
	typedef AActor Super;

public:
	virtual void Tick_PrePhysics(float Delta) override;

private:
	UINT64 mGeneration;

	std::vector<TWeakPtr<AActor>> mActorPool;

	size_t mPoolUsage = 0;
};

class WProjectileAnimWorld : public WDefaultWorld
{
	typedef WDefaultWorld Super;
public:
	WProjectileAnimWorld();

	virtual void Tick(float Delta) override;

private	:
	float mElapsedTime = 0;

	std::vector<TWeakPtr<AProjectileAnimActor>> mProjs;
};
