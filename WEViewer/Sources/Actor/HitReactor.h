#pragma once

#include "Actor/PhysicsBox.h"
#include "GameFramework/Interface/HitInterface.h"

class AHitReactor : public APhysicsBox, public IHitInterface
{
	virtual void OnHit(WSceneComponent* Instigator, WPhysicsComponent* HittedComponent, XMFLOAT3 ImpulseDir, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance, float Damage) override;
};

REGISTER_ACTOR(AHitReactor);