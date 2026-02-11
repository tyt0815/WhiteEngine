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

bool WActorComponent::HasTag(const std::string& Tag, bool bCheckOwner)
{
	return WObject::HasTag(Tag) || (bCheckOwner && mOwner->HasTag(Tag));
}

void WActorComponent::BeginComponent()
{
	if (IsActivate())
	{
		Activate();
	}
}
