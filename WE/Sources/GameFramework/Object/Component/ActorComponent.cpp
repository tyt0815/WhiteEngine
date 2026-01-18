#include "ActorComponent.h"
#include "GameFramework/Object/Actor/Actor.h"

void WActorComponent::SetOwner(TWeakPtr<AActor> Owner)
{
	mOwner = Owner;
}

WWorld* WActorComponent::GetWorld() const
{
	return !mOwner.expired()  ? mOwner.lock()->GetWorld() : nullptr;
}
