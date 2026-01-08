#include "Bullet.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "GameFramework/Object/Component/ProjectileMovementComponent.h"

ABullet::ABullet()
{
	StaticMeshComp = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(StaticMeshComp);
	StaticMeshComp->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_ScuffedGoldSphere));
	StaticMeshComp->SetLocalScale({ 0.3f, 0.3f , 0.3f });

	ProjComp->mVelocity = { 0.0f, 0.0f, 20.0f };
}
