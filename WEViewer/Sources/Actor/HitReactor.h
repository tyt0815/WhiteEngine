#pragma once

#include "Actor/PhysicsBox.h"
#include "Interface/HitInterface.h"

class AHitReactor : public APhysicsBox, public IHitInterface
{
	virtual void OnHit(WSceneComponent* Instigator, WPhysicsComponent* HittedComp, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Damage) override;
};

REGISTER_ACTOR(AHitReactor);