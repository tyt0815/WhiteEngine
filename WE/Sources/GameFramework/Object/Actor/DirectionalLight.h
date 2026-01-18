#pragma once
#include "Actor.h"

class WDirectionalLightComponent;

class ADirectionalLight : public AActor
{
public:
	ADirectionalLight();

private:
	TWeakPtr<WDirectionalLightComponent> mDirectionalLightComponent;

public:
	inline TWeakPtr<WDirectionalLightComponent> GetDirLightComp() const
	{
		return mDirectionalLightComponent;
	}
};