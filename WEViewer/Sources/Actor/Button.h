#pragma once

#include "Actor/Actor.h"
#include "Interface/InteractionInterface.h"
#include "GameFramework/Interface/HitInterface.h"

class WBoxComponent;
class WStaticMeshComponent;

DECLARE_MULTICAST_DELEGATE(FOnButtonInteracted);

class AButton : public AActor, public IInteractionInterface, public IHitInterface
{
public:
	AButton();

	virtual void Interaction() override;

	virtual void OnBeginInteractionFocus() override;

	virtual void OnEndInteractionFocus() override;

	virtual void OnHit(WSceneComponent* Instigator, WPhysicsComponent* HittedComponent, XMFLOAT3 ImpulseDir, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance, float Damage) override;

public:
	FOnButtonInteracted mOnButtonInteracted;

private:
	WBoxComponent* mBoxComp;

	WStaticMeshComponent* mStaticMeshComp;
};