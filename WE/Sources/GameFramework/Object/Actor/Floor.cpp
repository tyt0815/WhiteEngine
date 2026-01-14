#include "Floor.h"
#include "GameFramework/Object/World/World.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "GameFramework/Object/Component/SplineComponent.h"

AFloor::AFloor()
{
	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_DefaultFloor));

	mBoxPhysicsExtent = { 50, 0.5f, 50 };

	mObjectType = EObjectType::EOT_Static;
	mActorPhysicsShape = EPhysicsShape::EPS_Box;
	mbPhysicSimulate = true;
}

void AFloor::BeginPlay()
{
	Super::BeginPlay();
}