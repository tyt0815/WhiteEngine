#pragma once
#include "Utility/Delegate.h"
#include <DirectXMath.h>

class AActor;
class WPhysicsComponent;
using namespace DirectX;

// Actor, Component, ImpactPoint, Normal, Distance;
DECLARE_MULTICAST_DELEGATE_FiveParams(FOnCollision, AActor*, WPhysicsComponent*, XMFLOAT3, XMFLOAT3, float);

class ICollisionGenerator
{
public:
	virtual ~ICollisionGenerator() = default;

	virtual void GenerateCollision() = 0;


};

class FCollisionGeneratorBase : public ICollisionGenerator
{
public:
	FOnCollision mOnCollision;
};