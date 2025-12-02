#pragma once
#include "GameFramework/Object/Actor/Actor.h"

class AScuffedGoldBox : public AActor
{
public:
	AScuffedGoldBox();

	virtual void Tick(float Seconds) override;

private:
	float Alpha = 0;
};