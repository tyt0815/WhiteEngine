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

TWeakPtr<AActor> WActorComponent::GetOwner() const
{
	return mOwner->GetWeakPtr<AActor>();
}

void WActorComponent::BeginComponent()
{
	if (IsActivate())
	{
		Activate();
	}
}
