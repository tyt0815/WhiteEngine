#pragma once

#include "Actor.h"
#include "Utility/Delegate.h"
#include "GameFramework/Interface/HitInstigator.h"

class WStateMachineComponent;

class AStateMachineActor : public AActor, public IHitInstigator
{
	typedef AActor Super;
public:
	AStateMachineActor();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSecond) override;

	virtual void LoadBlueprint(const FBlueprintAsset* Asset) override;

	virtual void OnDestroy() override;

	virtual void Hit(float Damage) override;

	virtual void Explosion(float Damage, float Radius) override;

private:
	void OnHit_Global(WSceneComponent* Instigator, WPhysicsComponent* HittedComponent, XMFLOAT3 ImpulseDir, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance);

	WStateMachineComponent* mStateMachine;

	bool mbNeedHit = false;

	float mHitDamage = 0;

	bool mbNeedExplosion = false;

	float mExplosionDamage = 0;

	float mExplosionRadius = 1;
};

REGISTER_ACTOR(AStateMachineActor);