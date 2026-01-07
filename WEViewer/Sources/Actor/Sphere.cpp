#include "Sphere.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

ASphere::ASphere()
{
	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_RustedIron2Sphere));
}
