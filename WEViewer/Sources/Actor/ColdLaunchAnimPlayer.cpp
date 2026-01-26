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
		mPlayTime += DeltaSecond * 2;

		if (auto AnimComp = mAnimComp.lock())
		{
			XMFLOAT3 ProjLoc = AnimComp->SampleAnimWorldLocationBySecond(mAnimSampler, mPlayTime);
			Proj->SetActorLocation(ProjLoc);			
			
			XMFLOAT3 ProjRot;
			XMVECTOR AxisV = XMLoadFloat3(&mRotationAxis);
			float Alpha = mRotationSampler.SampleAnimDataByFrame(AnimComp->SecondToFrame(mPlayTime)) / -90.0f;
			
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

void AColdLaunchAnimPlayer::PlayAnim(XMFLOAT3 TargetPos)
{
	mPlayTime = 0;
	mProjectile = GetWorld()->SpawnActor<ATopAttackMissile>();
	if (auto Proj = mProjectile.lock())
	{
		Proj->SetActorLocation(GetActorLocation());
		Proj->SetTargetPosition(TargetPos);

		auto Marker = Proj->GetCurrentHomingTarget().lock();
		if (Marker == nullptr)
		{
			mRotationRadian = 0;
			mRotationAxis = XMFLOAT3(0, 0, 0);
			return;
		}

		XMFLOAT3 Forward = GetFowardVector();
		XMVECTOR ForwardV = XMLoadFloat3(&Forward);
		XMFLOAT3 CurrPos = GetActorLocation();
		XMFLOAT3 MarkerPos = Marker->GetActorLocation();
		XMVECTOR InjectionDirV = XMVector3Normalize(
			XMVectorSubtract(XMLoadFloat3(&MarkerPos), XMLoadFloat3(&CurrPos))
		);

		mRotationRadian = XMVectorGetX(XMVector3AngleBetweenNormals(ForwardV, InjectionDirV));

		XMVECTOR AxisV = XMVectorZero();
		if (mRotationRadian > 0.0001f)
		{
			AxisV = XMVector3Cross(ForwardV, InjectionDirV);
		}
		if (XMVector3Equal(AxisV, XMVectorZero()))
		{
			XMFLOAT3 Right = GetRightVector();
			XMVECTOR RightV = XMLoadFloat3(&Right);
			AxisV = XMVector3Cross(RightV, AxisV);
		}

		XMStoreFloat3(&mRotationAxis, XMVector3Normalize(AxisV));
	}
}