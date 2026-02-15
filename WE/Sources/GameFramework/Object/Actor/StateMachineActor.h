#pragma once

#include "Actor.h"

class WStateMachineComponent;

class AStateMachineActor : public AActor
{
	typedef AActor Super;
public:
	AStateMachineActor();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSecond) override;

	virtual void LoadBlueprint(const FBlueprintAsset* Asset) override;

private:
	TWeakPtr<WStateMachineComponent> mStateMachineComp;
};

REGISTER_ACTOR(AStateMachineActor);