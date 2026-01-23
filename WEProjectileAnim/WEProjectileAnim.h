#pragma once

#include "World/DefaultWorld.h"
#include "Actor/Actor.h"
#include "Component/ProjectileMovementComponent.h"
#include <queue>
#include <array>

class WObjectAnimComponent;
class WStaticMeshComponent;
class WBoxComponent;
class FObjectAnimSampler;

class ARing : public AActor
{
public:
	ARing();

private:
	TWeakPtr<WStaticMeshComponent> mStaticMeshComp;
};


class AProjectile : public AActor
{
	typedef AActor Super;
public:
	AProjectile();

	virtual void BeginPlay() override;

	virtual void Tick(float Delta) override;

private:
	TWeakPtr<WStaticMeshComponent> mStaticMeshComp;

	TWeakPtr<WBoxComponent> mBoxCollision;

	TWeakPtr<WProjectileMovementComponent> mProjComp;

	TWeakPtr<ARing> mRing;

	TWeakPtr<WObjectAnimComponent> mObjectAnimComp;

	FObjectAnimSampler* mProjAnimSampler;

	float mElapsedTime = 0;
};

class AProjectileAnimActor : public AActor
{
	typedef AActor Super;
public:
	AProjectileAnimActor();

	virtual void BeginPlay() override;

	virtual void Tick(float Delta) override;

private:
	TWeakPtr<WObjectAnimComponent> mObjectAnimComp;

	TWeakPtr<AProjectile> mProjs[3];

	FObjectAnimSampler* mProjAnimSamplers[3];

	float mElapsedTime = 0;
};

class AProjSpawner : public AActor
{
	typedef AActor Super;

private:
	UINT64 mGeneration;

	std::vector<TWeakPtr<AActor>> mActorPool;

	size_t mPoolUsage = 0;
};

class WProjectileAnimWorld : public WDefaultWorld
{
	typedef WDefaultWorld Super;
public:
	virtual void BeginPlay() override;

private	:
	float mElapsedTime = 0;

	std::vector<TWeakPtr<AProjectileAnimActor>> mProjs;
};
