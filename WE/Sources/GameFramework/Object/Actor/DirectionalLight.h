#pragma once
#include "Actor.h"

class WDirectionalLightComponent;

class ADirectionalLight : public AActor
{
public:
	ADirectionalLight();

private:
	WDirectionalLightComponent* DirectionalLightComponent = nullptr;
};