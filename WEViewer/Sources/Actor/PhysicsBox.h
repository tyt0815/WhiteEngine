#pragma once
#include "Actor/Actor.h"
#include "Component/BoxComponent.h"
#include "Component/StaticMeshComponent.h"

class APhysicsBox : public AActor
{
	typedef AActor Super;

public:
	APhysicsBox();

private:
	TWeakPtr<WBoxComponent> mBoxComp;

	TWeakPtr<WStaticMeshComponent> mStaticMeshComp;
};