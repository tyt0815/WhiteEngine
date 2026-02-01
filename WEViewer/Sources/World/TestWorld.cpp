#include "TestWorld.h"
#include "Actor/PhysicsBox.h"
#include "Actor/HitReactor.h"
#include "Actor/ProjectileBase.h"
#include "Actor/HomingProjectile.h"
#include <algorithm>

void WTestWorld::BeginPlay()
{	
	Super::BeginPlay();

	if (auto Proj = SpawnActor<AHomingProjectile>().lock())
	{
		Proj->SetActorLocation(XMFLOAT3(0, 0, 10));
		if (auto Target = SpawnActor<AActor>().lock())
		{
			Target->SetActorLocation(XMFLOAT3(0, 10, 10));
			Proj->SetHomingTarget(Target->GetRootComponent());
		}
	}
}

void WTestWorld::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);
}