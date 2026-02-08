#include "ActorComponent.h"
#include "GameFramework/Object/Actor/Actor.h"
#include "World/World.h"

WActorComponent::WActorComponent()
{
}

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

TWeakPtr<AActor> WActorComponent::GetOwner() const
{
	return mOwner->GetWeakPtr<AActor>();
}

void WActorComponent::BeginComponent()
{
	OnActivate();
}
