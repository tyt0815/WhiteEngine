#include "Skull.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

ASkull::ASkull()
{
	WStaticMeshComponent* Component = CreateSceneComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_Skull));
}
