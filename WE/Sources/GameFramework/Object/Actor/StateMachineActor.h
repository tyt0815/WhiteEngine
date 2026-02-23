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
	void OnHit_Global(WSceneComponent* Instigator, WPhysicsComponent* HittedComponent, XMFLOAT3 ImpulseDir, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance, float Damage);

	WStateMachineComponent* mStateMachine;
};

REGISTER_ACTOR(AStateMachineActor);