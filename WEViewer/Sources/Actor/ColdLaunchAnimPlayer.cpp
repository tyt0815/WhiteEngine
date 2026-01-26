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
			
			XMFLOAT3 ProjRot;
			XMVECTOR AxisV = XMLoadFloat3(&mRotationAxis);
			float Alpha = mRotationSampler.SampleAnimDataByFrame(AnimComp->SecondToFrame(mPlayTime)) / -90.0f;
			if (mPlayTime > AnimComp->GetDuration())
			{
				int a = 0;
			}
			if (mRotationRadian > 0.0001f && XMVector3NotEqual(AxisV, XMVectorZero()))
			{
				XMVECTOR RotationQuatV = XMQuaternionRotationAxis(AxisV, mRotationRadian * Alpha);

				XMFLOAT3 Forward = GetFowardVector();
				XMVECTOR NewForwardV = XMVector3Rotate(XMLoadFloat3(&Forward), RotationQuatV);
				XMFLOAT3 Up = GetUpVector();
				XMVECTOR NewUpV = XMVector3Rotate(XMLoadFloat3(&Up), RotationQuatV);
				XMFLOAT3 Right = GetRightVector();
				XMVECTOR NewRightV = XMVector3Rotate(XMLoadFloat3(&Right), RotationQuatV);

				ProjRot = FDXMath::GetEulerRotationFromVectors(NewForwardV, NewRightV, NewUpV);
			}
			else
			{
				ProjRot = GetActorRotation();
			}

			Proj->SetActorRotation(ProjRot);

			if (mPlayTime > AnimComp->GetDuration())
			{
				mProjectile.reset();
				Destroy();
			}
		}		
	}
}