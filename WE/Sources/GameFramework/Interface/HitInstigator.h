#pragma once

class IHitInstigator
{
public:
	virtual void Hit(float Damage) = 0;

	virtual void Explosion(float Damage, float Radius) = 0;
};