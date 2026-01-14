#pragma once

#include "GameFramework/Object/Actor/Actor.h"

#include "Physics/PhysicsCore.h"

class AFloor : public AActor
{
	typedef AActor Super;
public:
	AFloor();

	virtual void BeginPlay() override;

private:
	class WBoxComponent* mBoxComp;

	class WStaticMeshComponent* StaticMeshComp;
};