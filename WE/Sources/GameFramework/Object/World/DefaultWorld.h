#pragma once
#include "World.h"

class WDefaultWorld : public WWorld
{
	typedef WWorld Super;
public:

	virtual void BeginPlay() override;
};