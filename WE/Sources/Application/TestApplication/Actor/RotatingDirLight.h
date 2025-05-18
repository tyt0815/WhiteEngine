#pragma once
#include "GameFramework/Object/Actor/DirectionalLight.h"

class ARotatingDirLight : public ADirectionalLight
{
	typedef ADirectionalLight Super;
public:
	virtual void Tick(float Delta) override;
};