#pragma once

#include "Sphere.h"

class AWaterBall : public ASphere
{
	using Super = ASphere;
public:
	AWaterBall();
	virtual void Tick(float Delta) override;
};