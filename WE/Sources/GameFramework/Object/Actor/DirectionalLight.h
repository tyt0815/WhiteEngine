#pragma once
#include "Actor.h"

class WDirectionalLightComponent;

class ADirectionalLight : public AActor
{
public:
	ADirectionalLight();

private:
	WDirectionalLightComponent* mDirectionalLightComponent = nullptr;

public:
	inline WDirectionalLightComponent* GetDirLightComp() const
	{
		return mDirectionalLightComponent;
	}
};