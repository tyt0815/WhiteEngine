#pragma once

#include "GameFrameWork/Object/Actor/Actor.h"

class AProjectileSpawner : public AActor
{
	typedef AActor Super;
public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void SpawnRandomProjectile();

	void SpawnBullet();

	float mCoolDown = 0;
};