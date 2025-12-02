#include "ThickMortarStonework.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

AThickMortarStonework::AThickMortarStonework()
{
	WStaticMeshComponent* Component = CreateSceneComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_ThickMortarStonework));
}

void AThickMortarStonework::Tick(float Seconds)
{
	AActor::Tick(Seconds);

	Alpha += Seconds;

	DirectX::XMFLOAT3 Location = GetActorLocation();
	Location.y = sin(Alpha);
	SetActorLocation(Location);
}
