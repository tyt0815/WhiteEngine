#include "LaminateFloorBrown.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

ALaminateFloorBrown::ALaminateFloorBrown()
{
	WStaticMeshComponent* Component = CreateSceneComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_LaminateFloorBrown));
}