#pragma once
#include "Actor/Actor.h"
#include "World/World.h"
#include "Component/ObjectAnimComponent.h"

class AColdLaunchAnimPlayer : public AActor
{
	typedef AActor Super;
public:
	AColdLaunchAnimPlayer();

	virtual void Tick(float DeltaSecond) override;

public:

	template<typename TProjtile>
	void PlayAnim();

private:
	TWeakPtr<WObjectAnimComponent> mAnimComp;
	
	FObjectAnimSampler* mAnimSampler;

	TWeakPtr<AActor> mProjectile;

	TWeakPtr<AActor> mCovers[4];

	float mPlayTime = 0;
};

template<typename TProjtile>
inline void AColdLaunchAnimPlayer::PlayAnim()
{
	mProjectile = GetWorld()->SpawnActor<TProjtile>();
	if (auto Proj = mProjectile.lock())
	{
		Proj->SetActorLocation(GetActorLocation());
	}
	mPlayTime = 0;
}