#pragma once

#include "Actor.h"
#include "Component/StaticMeshComponent.h"

class ABlueprintActor : public AActor
{
public:
	ABlueprintActor();

	TWeakPtr<WStaticMeshComponent> mStaticMeshComp;

	float Value = 0;
};