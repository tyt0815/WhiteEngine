#include "ProjectileMovementComponent.h"
#include "GameFramework/Object/Actor/Actor.h"

void WProjectileMovementComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);

	mLifeTimeElapsed += DeltaTime;
	if (mLifeTimeElapsed > mLiftSpan)
	{
		GetOwner()->Destroy();
	}
}
