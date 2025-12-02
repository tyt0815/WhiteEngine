#pragma once
#include "GameFramework/Object/Actor/Actor.h"

class AScuffedGoldSphere : public AActor
{
public:
	AScuffedGoldSphere();

	virtual void Tick(float Seconds) override;

private:
	float Alpha = 0;
};