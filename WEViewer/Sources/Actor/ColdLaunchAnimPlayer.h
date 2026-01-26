#pragma once
#include "Actor/Actor.h"
#include "World/World.h"
#include "Component/ObjectAnimComponent.h"
#include "TopAttackMissile.h"

class AColdLaunchAnimPlayer : public AActor
{
	typedef AActor Super;
public:
	AColdLaunchAnimPlayer();

	virtual void Tick(float DeltaSecond) override;

public:

	template<typename TProjtile>
	void PlayAnim(XMFLOAT3 TargetPos);

private:
	TWeakPtr<WObjectAnimComponent> mAnimComp;
	
	FObjectAnimSampler* mAnimSampler;

	FCurveSampler mLocationZSampler;

	FCurveSampler mRotationSampler;

	TWeakPtr<ATopAttackMissile> mProjectile;

	TWeakPtr<AActor> mCovers[4];

	XMFLOAT3 mRotationAxis;

	float mRotationRadian;

	float mPlayTime = 0;
};

template<typename TProjtile>
inline void AColdLaunchAnimPlayer::PlayAnim(XMFLOAT3 TargetPos)
{
	mPlayTime = 0;
	mProjectile = GetWorld()->SpawnActor<TProjtile>();
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