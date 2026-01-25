#include "TestWorld.h"
#include "Actor/TopAttackMissile.h"

void WTestWorld::BeginPlay()
{
	Super::BeginPlay();

	mMissileSystem = SpawnActor<AMissileSwarmSystem>();
	if (auto MissileSystem = mMissileSystem.lock())
	{
		MissileSystem->SetActorLocation(XMFLOAT3(0, 0, 10));
	}
}

void WTestWorld::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	mElapsedTime += DeltaSecond;
	if (mElapsedTime > 3)
	{
		if (auto MissileSystem = mMissileSystem.lock())
		{
			XMFLOAT3 TargetOrigin = MissileSystem->GetActorLocation();
			TargetOrigin.z += 5;
			// TargetOrigin.x += 10;
			MissileSystem->Fire<AColdLaunchAnimPlayer, ATopAttackMissile>(1, 1, TargetOrigin);
			mElapsedTime = 0;
		}
	}

	XMFLOAT3 Start(0, 0, 0);
	for (int i = 0; i < 10; ++i)
	{
		XMFLOAT3 End(0, -1.0f * (float)(i + 1), (float)i);
		FHitResult HitResult;
		LineTrace(Start, End, HitResult, true);
	}
}
