#include "Sphere.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

ASphere::ASphere():
	Super()
{
	WStaticMeshComponent* Component = CreateSceneComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_Sphere));
}
