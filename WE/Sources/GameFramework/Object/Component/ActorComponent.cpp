#include "ActorComponent.h"
#include "GameFramework/Object/Actor/Actor.h"
#include "World/World.h"

void WActorComponent::BeginComponent()
{
	OnActivate();
}

void WActorComponent::SetOwner(TWeakPtr<AActor> Owner)
{
	mOwner = Owner;
}

void WActorComponent::OnDestroy()
{
	OnDeactivate();
}

void WActorComponent::OnActivate()
{
	GetWorld()->EnqueueComponentTick(this);
}

void WActorComponent::OnDeactivate()
{
	GetWorld()->DequeueComponentTick(this);
}