#include "RustedIron2Sphere.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

ARustedIron2Sphere::ARustedIron2Sphere()
{
	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_RustedIron2Sphere));
}

void ARustedIron2Sphere::Tick(float Seconds)
{
	AActor::Tick(Seconds);

	Alpha += Seconds;

	DirectX::XMFLOAT3 Location = GetActorLocation();
	Location.y = -sin(Alpha);
	SetActorLocation(Location);
}
