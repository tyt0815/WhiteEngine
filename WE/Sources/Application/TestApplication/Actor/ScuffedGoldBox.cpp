#include "ScuffedGoldBox.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

AScuffedGoldBox::AScuffedGoldBox()
{
	WStaticMeshComponent* Component = CreateSceneComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_ScuffedGoldBox));
}
