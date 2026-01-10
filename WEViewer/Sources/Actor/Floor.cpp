#include "Floor.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

AFloor::AFloor()
{
	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_LaminateFloorBrown));

	mBody = CreateBoxBody({10, 10, 10});
}
