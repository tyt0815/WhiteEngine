#include "Floor.h"
#include "GameFramework/Object/World/World.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "GameFramework/Object/Component/SplineComponent.h"
#include "Component/BoxComponent.h"

AFloor::AFloor()
{
	mBoxComp = CreateComponent<WBoxComponent>();
	SetRootComponent(mBoxComp);
	mBoxComp->ActivatePhysicBody();
	mBoxComp->SetExtent(XMFLOAT3(50, 0.5f, 50));
	mBoxComp->SetObjectType(EObjectType::EOT_Static);

	StaticMeshComp = CreateComponent<WStaticMeshComponent>();
	StaticMeshComp->SetupAttachment(GetRootComponent());
	StaticMeshComp->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_DefaultFloor));
}

void AFloor::BeginPlay()
{
	Super::BeginPlay();
}