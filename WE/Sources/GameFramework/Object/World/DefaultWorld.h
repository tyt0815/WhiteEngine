#pragma once
#include "World.h"

class WDefaultWorld : public WWorld
{
	typedef WWorld Super;
public:
	WDefaultWorld();

	virtual void Tick(float DeltaTime) override;
};