#include "ActorComponent.h"
#include "GameFramework/Object/Actor/Actor.h"
#include "World/World.h"

void WActorComponent::Destroy()
{
	OnDestroy();
}

void WActorComponent::Activate()
{
	OnActivate();
}

void WActorComponent::Deactivate()
{
	OnDeactivate();
}

void WActorComponent::BeginComponent()
{
	OnActivate();
}

void WActorComponent::SetOwner(TWeakPtr<AActor> Owner)
{
	mOwner = Owner;
}