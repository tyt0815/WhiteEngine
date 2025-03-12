#include "Actor.h"
#include "GameFramework/Object/Component/SceneComponent.h"

void AActor::Tick(float Delta)
{
}

void AActor::SetRootComponent(WSceneComponent* Component)
{
	if (mRootComponent == Component)
	{
		return;
	}
	if (mRootComponent != nullptr)
	{
		mRootComponent->SetupAttachment(Component);
		GetWObjectManager()->RemoveRootComponent(mRootComponentPoolId);
	}

	mRootComponent = Component;
	mRootComponentPoolId = GetWObjectManager()->RegisterRootComponent(mRootComponent);
}

void AActor::SetupComponent(WActorComponent* Component)
{
	Component->SetOwner(this);
}

void AActor::SetupSceneComponent(WSceneComponent* Component)
{
	if (mRootComponent)
	{
		Component->SetupAttachment(mRootComponent);
	}
	else
	{
		SetRootComponent(Component);
	}
}