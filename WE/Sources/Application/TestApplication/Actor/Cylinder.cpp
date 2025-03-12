#include "Cylinder.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

ACylinder::ACylinder()
{
	WStaticMeshComponent* Component = CreateSceneComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_Cylinder));
}