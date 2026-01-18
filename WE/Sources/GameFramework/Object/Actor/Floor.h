#pragma once

#include "GameFramework/Object/Actor/Actor.h"
#include "Component/BoxComponent.h"
#include "Component/StaticMeshComponent.h"

class AFloor : public AActor
{
	typedef AActor Super;
public:
	AFloor();

	virtual void BeginPlay() override;

private:
	TWeakPtr<WBoxComponent> mBoxComp;

	TWeakPtr<WStaticMeshComponent> mStaticMeshComp;
};