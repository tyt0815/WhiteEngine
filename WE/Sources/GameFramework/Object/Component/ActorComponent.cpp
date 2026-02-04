#include "ActorComponent.h"
#include "GameFramework/Object/Actor/Actor.h"
#include "World/World.h"

WActorComponent::WActorComponent()
{
	RegisterWProperty("Owner", mOwner);
	mBeginComponentEvent = RegisterWEvent("BeginComponent");
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
	mBeginComponentEvent->Dispatch();
}

void WActorComponent::LoadBlueprint(WObject* Context, const BlueprintAsset::FComponentNode* RootNode)
{
	LoadWProperties(RootNode->Properties);
	LoadWVariables(RootNode->Variables);
	LoadEvents(Context, RootNode->Events);
}
