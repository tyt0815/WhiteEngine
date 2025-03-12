#pragma once

#include "GameFramework/Object/World/World.h"

class WBoxWorld : public WWorld
{
	using Super = WWorld;
public:
	WBoxWorld();

	virtual void BuildWorldActors();
};