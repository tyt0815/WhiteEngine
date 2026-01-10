#include "Floor.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

AFloor::AFloor()
{
	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_LaminateFloorBrown));

	mBody = CreateBoxBody({100, 1, 100}, EObjectType::EOT_Static, false);
	mBody->AddBody();
	mBody->SetActivate(false);
}

void AFloor::BeginPlay()
{
	Super::BeginPlay();

	mBody->SetPosition({ 0.0f, -3.0f, 0.0f });
}

void AFloor::Tick(float Delta)
{
	Super::Tick(Delta);

	SetActorTransform(mBody->GetTransform());
}
