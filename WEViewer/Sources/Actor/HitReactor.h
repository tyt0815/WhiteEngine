#pragma once

#include "Actor/Actor.h"
#include "GameFramework/Interface/HitInterface.h"

class WPhysicsComponent;

class AHitReactor : public AActor, public IHitInterface
{
public:
	AHitReactor();

	virtual void OnHit(WSceneComponent* Instigator, WPhysicsComponent* HittedComponent, XMFLOAT3 ImpulseDir, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance, float Damage) override;


protected:

	WPhysicsComponent* mPhysicsComp;
};

REGISTER_ACTOR(AHitReactor);