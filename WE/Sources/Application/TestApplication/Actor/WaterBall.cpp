#include "WaterBall.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

AWaterBall::AWaterBall()
{
	WStaticMeshComponent* Component = CreateSceneComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_WaterBall));
}
