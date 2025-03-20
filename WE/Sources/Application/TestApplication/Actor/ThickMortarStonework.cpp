#include "ThickMortarStonework.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

AThickMortarStonework::AThickMortarStonework()
{
	WStaticMeshComponent* Component = CreateSceneComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_ThickMortarStonework));
}
