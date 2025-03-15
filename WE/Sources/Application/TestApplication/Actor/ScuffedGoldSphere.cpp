#include "ScuffedGoldSphere.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

AScuffedGoldSphere::AScuffedGoldSphere()
{
	WStaticMeshComponent* Component = CreateSceneComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_ScuffedGoldSphere));
}
