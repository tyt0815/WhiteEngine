#include "ActorComponent.h"
#include "GameFramework/Object/Actor/Actor.h"
#include "World/World.h"

WActorComponent::WActorComponent()
{
	mBeginComponentEvent = GetEvent("BeginComponent");
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

void WActorComponent::BeginComponent()
{
	OnActivate();
	mBeginComponentEvent->Dispatch();
}

void WActorComponent::LoadBlueprint(const BlueprintAsset::FComponentNode* RootNode)
{
	LoadWProperties(RootNode->Properties);
	LoadEvents(RootNode->Events);
}
