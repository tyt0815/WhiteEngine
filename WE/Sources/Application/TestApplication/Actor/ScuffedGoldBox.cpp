#include "ScuffedGoldBox.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

AScuffedGoldBox::AScuffedGoldBox()
{
	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_ScuffedGoldBox));
}

void AScuffedGoldBox::Tick(float Seconds)
{
	AActor::Tick(Seconds);

	Alpha += Seconds;

	DirectX::XMFLOAT3 Location = GetActorLocation();
	Location.y = -sin(Alpha);
	SetActorLocation(Location);
}
