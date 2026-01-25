#include "ColdLaunchAnimPlayer.h"

AColdLaunchAnimPlayer::AColdLaunchAnimPlayer()
{
	mAnimComp = CreateComponent<WObjectAnimComponent>();
	if(auto Comp = mAnimComp.lock())
	{
		Comp->SetupAttachment(GetRootComponent());
		Comp->LoadKeyframesFromOADAsset(L"OAD_ColdLaunch");
		mAnimSampler = Comp->GetObjectAnimSampler("Projectile");
		mLocationZSampler = mAnimSampler->GetCurveSampler("LocationZ");
		mRotationSampler = mAnimSampler->GetCurveSampler("RotationX");
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
			XMFLOAT3 ProjLoc = AnimComp->SampleAnimWorldLocationBySecond(mAnimSampler, mPlayTime);
			Proj->SetActorLocation(ProjLoc);

			XMFLOAT3 ProjRot(0,0,0);
			ProjRot.z = mRotationSampler.SampleAnimDataByFrame(AnimComp->SecondToFrame(mPlayTime));
			Proj->SetActorRotation(ProjRot);

			if (mPlayTime > AnimComp->GetDuration())
			{
				mProjectile.reset();
				Destroy();
			}
		}		
	}
}