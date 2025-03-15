#include "RustedIron2Sphere.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

ARustedIron2Sphere::ARustedIron2Sphere()
{
	WStaticMeshComponent* Component = CreateSceneComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_RustedIron2Sphere));
}
