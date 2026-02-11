#pragma once
#include <DirectXMath.h>

using XMFLOAT3 = DirectX::XMFLOAT3;

class AActor;
class WPhysicsComponent;

class IHitInterface
{
public:
	virtual void OnHit(WSceneComponent* Instigator, WPhysicsComponent* HittedComp, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Damage) = 0;
};