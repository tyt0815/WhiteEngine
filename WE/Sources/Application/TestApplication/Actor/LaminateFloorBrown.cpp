#include "LaminateFloorBrown.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "DirectX/DXMath.h"

ALaminateFloorBrown::ALaminateFloorBrown()
{
	WStaticMeshComponent* Component = CreateSceneComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_LaminateFloorBrown));
}

void ALaminateFloorBrown::Tick(float Delta)
{
	AActor::Tick(Delta);
}
