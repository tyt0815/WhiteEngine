#pragma once

#include "World.h"

class WBoxWorld : public WWorld
{
	using Super = WWorld;
public:
	WBoxWorld();

	virtual void BuildWorldActors();
};