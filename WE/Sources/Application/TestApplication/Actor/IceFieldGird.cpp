#include "IceFieldGird.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

AIceFieldGrid::AIceFieldGrid()
{
	WStaticMeshComponent* Component = CreateSceneComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_IceFieldGrid));
}
