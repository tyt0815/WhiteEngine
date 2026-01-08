#include "ProjectileBox.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "GameFramework/Object/Component/ProjectileMovementComponent.h"

AProjectileBox::AProjectileBox()
{
	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_ScuffedGoldBox));
}
