#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include "Actor/Actor.h"
#include "Component/PhysicsComponent.h"
#include "Utility/Container.h"

struct FHitResult
{
	inline void SetActorAndHitComponent(TWeakPtr<WPhysicsComponent> InHitComponent)
	{
		HitComponent = InHitComponent;
		Actor = HitComponent.lock()->GetOwner().lock()->GetWeakPtr<AActor>();
	}

	TWeakPtr<AActor> Actor;
	TWeakPtr<WPhysicsComponent> HitComponent;
	XMFLOAT3 ImpactPoint;
	XMFLOAT3 Normal;
	float Distance;
};