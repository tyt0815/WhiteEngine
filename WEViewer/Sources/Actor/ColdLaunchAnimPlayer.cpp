#include "ColdLaunchAnimPlayer.h"

AColdLaunchAnimPlayer::AColdLaunchAnimPlayer()
{
	mAnimComp = CreateComponent<WObjectAnimComponent>();
	if(auto Comp = mAnimComp.lock())
	{
		Comp->LoadKeyframesFromOADAsset(L"OAD_ColdLaunch");
		mAnimSampler = Comp->GetObjectAnimSampler("Projectile");
	}

	mTickGroup = ETickGroup::ETG_PostPhysics;
}

void AColdLaunchAnimPlayer::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	if (auto Proj = mProjectile.lock())
	{
		mPlayTime += DeltaSecond;

		if (auto AnimComp = mAnimComp.lock())
		{
			Proj->SetActorTransform(AnimComp->SampleAnimWorldTransformBySecond(mAnimSampler, mPlayTime));

			if (mPlayTime > AnimComp->GetDuration())
			{
				mProjectile.reset();
				Destroy();
			}
		}		
	}
}