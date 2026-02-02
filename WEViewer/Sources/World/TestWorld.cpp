#include "TestWorld.h"
#include "Actor/PhysicsBox.h"
#include "Actor/HitReactor.h"
#include "Actor/ProjectileBase.h"
#include "Actor/HomingProjectile.h"
#include <algorithm>

void WTestWorld::BeginPlay()
{	
	Super::BeginPlay();

	
}

void WTestWorld::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);

	static float a = 32;
	a += DeltaSecond;

	if (a > 3)
	{
		if (auto Proj = SpawnActor<AHomingProjectile>(L"BP_Projectile").lock())
		{
			XMFLOAT3 Loc = XMFLOAT3(0, 0, 10);
			Proj->SetActorLocation(Loc);
			if (auto Target = SpawnActor<AActor>().lock())
			{
				Loc.y += 10;
				Loc.z -= 10;
				Target->SetActorLocation(Loc);
				Proj->SetHomingTarget(Target->GetRootComponent());
			}
		}
		a = 0;
	}
}