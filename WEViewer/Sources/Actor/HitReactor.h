#pragma once

#include "Actor/PhysicsBox.h"
#include "Interface/HitInterface.h"

class AHitReactor : public APhysicsBox, public IHitInterface
{
	virtual void OnHit(AActor* Instigator) override;
};