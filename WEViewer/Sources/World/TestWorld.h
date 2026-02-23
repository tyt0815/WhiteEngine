#pragma once

#include "GameFramework/Object/World/DefaultWorld.h"

class ADenseBoxHitReactorManager;

class WTestWorld : public WDefaultWorld
{
	typedef WDefaultWorld Super;
public:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSecond) override;

private:
	ADenseBoxHitReactorManager* mDenseBoxHitReactorManager;
};