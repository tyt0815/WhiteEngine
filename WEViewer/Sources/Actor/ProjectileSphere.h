#pragma once


#include "GameFramework/Object/Actor/Actor.h"

class AProjectileSphere : public AActor
{
	typedef AActor Super;
public:
	AProjectileSphere();

	virtual void Tick(float DeltaTime) override;

private:
	float ElapsedTime = 0;
};