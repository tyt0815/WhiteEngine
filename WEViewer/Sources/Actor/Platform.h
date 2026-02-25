#pragma once
#include "Actor/Actor.h"

class WStaticMeshComponent;
class WBoxComponent;

class APlatform : public AActor
{
	typedef AActor Super;
public:
	APlatform();

	virtual void BeginPlay() override;

public:
	void MovePlatform(const XMFLOAT3& Loc, bool bCarryPassengers = true);

private:
	WBoxComponent* mBoxComp;

	WStaticMeshComponent* mStaticMeshComp;

	void OnHit(WPhysicsComponent* OnHit, XMFLOAT3 ImpactPoint);

	void OnExitHit(WPhysicsComponent* OnHit);

	std::vector<TWeakPtr<AActor>> mContactedActor;
};