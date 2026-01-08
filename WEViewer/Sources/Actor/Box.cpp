#include "Box.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

ABox::ABox()
{
	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_ScuffedGoldBox));
}