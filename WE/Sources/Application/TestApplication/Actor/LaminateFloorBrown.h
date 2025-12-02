#pragma once
#include "GameFramework/Object/Actor/Actor.h"

class ALaminateFloorBrown : public AActor
{
public:
	ALaminateFloorBrown();

	virtual void Tick(float Delta) override;

	
private:
	float Alpha = 0;
};