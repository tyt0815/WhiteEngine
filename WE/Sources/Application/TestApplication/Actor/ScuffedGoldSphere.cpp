#include "ScuffedGoldSphere.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

AScuffedGoldSphere::AScuffedGoldSphere()
{
	WStaticMeshComponent* Component = CreateSceneComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	// Component->SetCastShadow(false);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_ScuffedGoldSphere));
}

void AScuffedGoldSphere::Tick(float Seconds)
{
	AActor::Tick(Seconds);

	Alpha += Seconds;

	DirectX::XMFLOAT3 Location = GetActorLocation();
	Location.y = sin(Alpha);
	SetActorLocation(Location);
}
