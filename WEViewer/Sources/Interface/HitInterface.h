#pragma once

class AActor;

class IHitInterface
{
public:
	virtual void OnHit(AActor* Instigator) = 0;
};