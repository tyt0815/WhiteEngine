#pragma once
#include "Component/PhysicsComponent.h"
#include "Utility/Container.h"

struct FHitResult
{
	TWeakPtr<WPhysicsComponent> HitComponent;
	XMFLOAT3 ImpactPoint;
};