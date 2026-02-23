#pragma once

#include "Actor/Actor.h"
#include "Interface/InteractionInterface.h"

class WBoxComponent;
class WStaticMeshComponent;

DECLARE_MULTICAST_DELEGATE(FOnButtonInteracted);

class AButton : public AActor, public IInteractionInterface
{
public:
	AButton();

	virtual void Interaction() override;

	virtual void OnBeginInteractionFocus() override;

	virtual void OnEndInteractionFocus() override;

public:
	FOnButtonInteracted mOnButtonInteracted;

private:
	WBoxComponent* mBoxComp;

	WStaticMeshComponent* mStaticMeshComp;
};