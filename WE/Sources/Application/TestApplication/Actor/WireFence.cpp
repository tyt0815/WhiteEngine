#include "WireFence.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

AWireFence::AWireFence()
{
	WStaticMeshComponent* Component = CreateSceneComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_WireFence));
}