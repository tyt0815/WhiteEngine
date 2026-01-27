#include "ColdLaunchAnimPlayer.h"

AColdLaunchAnimPlayer::AColdLaunchAnimPlayer()
{
	mAnimComp = CreateComponent<WObjectAnimComponent>();
	if(auto Comp = mAnimComp.lock())
	{
		Comp->SetupAttachment(GetRootComponent());
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
		mPlayTime += DeltaSecond * 2.0f;

		if (auto AnimComp = mAnimComp.lock())
		{

			FTransform ProjTransform = AnimComp->SampleAnimWorldTransformBySecond(mAnimSampler, mPlayTime);
			 Proj->SetActorTransform(ProjTransform);

			if (mPlayTime > AnimComp->GetDuration())
			{
				Proj->SetTargetPosition(mTargetPos);
				mProjectile.reset();
				Destroy();
			}
		}		
	}
}

void AColdLaunchAnimPlayer::PlayAnim(XMFLOAT3 TargetPos)
{
	mPlayTime = 0;
	mProjectile = GetWorld()->SpawnActor<ATopAttackMissile>();
	mTargetPos = TargetPos;
}