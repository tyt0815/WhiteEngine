#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include "Component/PhysicsComponent.h"
#include "Utility/Container.h"

struct FHitResult
{

	inline void SetActorAndHitComponent(TWeakPtr<WPhysicsComponent> InHitComponent)
	{
		HitComponent = InHitComponent;
		Actor = HitComponent.lock()->GetOwner();
	}

	TWeakPtr<AActor> Actor;
	TWeakPtr<WPhysicsComponent> HitComponent;
	XMFLOAT3 ImpactPoint;
	XMFLOAT3 Normal;
	float Distance;
};