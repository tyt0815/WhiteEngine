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

	XMFLOAT3 Start(0, 0, 0);
	for (int i = 0; i < 10; ++i)
	{
		XMFLOAT3 End(0, -1.0f * (float)(i + 1), (float)i);
		FHitResult HitResult;
		LineTrace(Start, End, HitResult, false);
	}
}
