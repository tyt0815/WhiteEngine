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
				XMFLOAT3 Target = Proj->GetActorLocation();
				Target.z -= 1;
				Target.y += 1;
				GetWorld()->DrawDebugLine(Proj->GetActorLocation(), Target, XMFLOAT4(1, 0, 0, 1), 2);
				Proj->SetTargetLocation(Target);
				mProjectile.reset();
				Destroy();
			}
		}		
	}
}