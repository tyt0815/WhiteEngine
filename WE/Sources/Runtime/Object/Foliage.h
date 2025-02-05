#pragma once

#include "Billboard.h"

class AFoliage : public ABillboard
{
	using Super = ABillboard;
public:
	AFoliage();

	virtual void Tick(float Delta) override;
};