#pragma once
#include "Actor/Actor.h"

class WStaticMeshComponent;
class WBoxComponent;

class APlatform : public AActor
{
public:
	APlatform();

	
private:
	WBoxComponent* mBoxComp;

	WStaticMeshComponent* mStaticMeshComp;
};