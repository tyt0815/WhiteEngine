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

bool WActorComponent::HasTag(const std::string& Tag, bool bCheckOwner)
{
	return WObject::HasTag(Tag) || (bCheckOwner && GetOwner<AActor>()->HasTag(Tag));
}

void WActorComponent::BeginComponent()
{
	if (IsActivate())
	{
		Activate();
	}
}
