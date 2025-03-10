#pragma once

#include "World.h"

class WTestWorld : public WWorld
{
	typedef WWorld Super;
public:
	WTestWorld();

protected:
	virtual void BuildWorldActors() override;
};