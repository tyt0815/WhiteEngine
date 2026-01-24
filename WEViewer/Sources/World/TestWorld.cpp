#include "TestWorld.h"
#include "Actor/TopAttackMissile.h"

void WTestWorld::BeginPlay()
{
	Super::BeginPlay();
}

void WTestWorld::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	mElapsedTime += DeltaSecond;
	if (mElapsedTime > 3)
	{
		if (auto MissileSystem = SpawnActor<AMissileSwarmSystem>().lock())
		{
			MissileSystem->Fire<AColdLaunchAnimPlayer, ATopAttackMissile>(1, 1);
			mElapsedTime = 0;
		}
	}
}
