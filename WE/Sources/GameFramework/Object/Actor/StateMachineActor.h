#pragma once

#include "Actor.h"
#include "Utility/Delegate.h"

class WStateMachineComponent;

class AStateMachineActor : public AActor
{
	typedef AActor Super;
public:
	AStateMachineActor();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSecond) override;

	virtual void LoadBlueprint(const FBlueprintAsset* Asset) override;

	virtual void OnDestroy() override;

private:
	void OnHit_Global();

	WStateMachineComponent* mStateMachine;
};

REGISTER_ACTOR(AStateMachineActor);