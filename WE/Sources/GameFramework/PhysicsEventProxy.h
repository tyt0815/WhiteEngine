#pragma once

#include <DirectXMath.h>
#include "Component/PhysicsComponent.h"
#include "Utility/Memory.h"

using namespace DirectX;

struct FPhysicEventInfo
{
	TWeakPtr<WPhysicsComponent> Comp1;
	TWeakPtr<WPhysicsComponent> Comp2;
	XMFLOAT3 ImpactPoint1;
	XMFLOAT3 ImpactPoint2;
};