#pragma once

#include "GameFramework/Object/World/DefaultWorld.h"
#include "Actor/MissileSwarmSystem.h"

class WTestWorld : public WDefaultWorld
{
	typedef WDefaultWorld Super;
public:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSecond) override;

private:
	float mElapsedTime = 0;
};