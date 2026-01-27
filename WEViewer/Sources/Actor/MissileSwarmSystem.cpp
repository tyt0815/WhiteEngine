#include "MissileSwarmSystem.h"

AMissileSwarmSystem::AMissileSwarmSystem()
{
	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_High);
}

void AMissileSwarmSystem::Tick(float Delta)
{
	Super::Tick(Delta);

	mElapsedTime += Delta;

	int r = (int)(mElapsedTime / mFireDelay);

	if (r != mLastFiredRow && r < mFireInfos.size())
	{
		mLastFiredRow = r;

		for (auto FireInfo : mFireInfos[r])
		{
			if (TSharedPtr<AColdLaunchAnimPlayer> AnimPlayer = FireInfo.AnimPlayer.lock())
			{
				AnimPlayer->PlayAnim(FireInfo.TargetPos);
			}
		}
	}
}
