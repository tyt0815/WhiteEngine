#include "ProjectileMovementComponent.h"
#include "GameFramework/Object/Actor/Actor.h"

void WProjectileMovementComponent::TickComponent_PrePhysics(float DeltaTime)
{
	Super::TickComponent_PrePhysics(DeltaTime);

	mLifeTimeElapsed += DeltaTime;
	if (mLifeSpan > 0 && mLifeTimeElapsed > mLifeSpan)
	{
		if (auto Owner = GetOwner().lock())
		{
			Owner->Destroy();
		}
	}
}

