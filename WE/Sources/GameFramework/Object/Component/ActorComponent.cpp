#include "ActorComponent.h"
#include "GameFramework/Object/Actor/Actor.h"

void WActorComponent::SetOwner(AActor* Owner)
{
	mOwner = Owner;
}

WWorld* WActorComponent::GetWorld() const
{
	return mOwner->GetWorld();
}
