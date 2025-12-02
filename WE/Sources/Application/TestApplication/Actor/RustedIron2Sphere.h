#pragma once
#include "GameFramework/Object/Actor/Actor.h"

class ARustedIron2Sphere : public AActor
{
public:
	ARustedIron2Sphere();

	virtual void Tick(float Seconds) override;

private:
	float Alpha = 0;
};