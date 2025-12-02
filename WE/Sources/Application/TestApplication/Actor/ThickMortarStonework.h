#pragma once
#include "GameFramework/Object/Actor/Actor.h"

class AThickMortarStonework : public AActor
{
public:
	AThickMortarStonework();

	virtual void Tick(float Seconds) override;

private:
	float Alpha = 0;
};