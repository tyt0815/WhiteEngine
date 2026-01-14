#pragma once

#include "GameFramework/Object/Actor/Actor.h"

#include "Physics/PhysicsCore.h"

class AFloor : public AActor
{
	typedef AActor Super;
public:
	AFloor();

	virtual void BeginPlay() override;

	virtual void Tick_PrePhysics(float Delta) override;

private:
	std::unique_ptr<FBody> mBody;

	class WSplineComponent* SplineComponent;
	AActor* SplineFollowingActor;

	float mCurrSplineDist = 0;
};