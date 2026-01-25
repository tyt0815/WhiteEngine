#include "Floor.h"
#include "GameFramework/Object/World/World.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "GameFramework/Object/Component/SplineComponent.h"
#include "Component/BoxComponent.h"

AFloor::AFloor()
{
	mBoxComp = CreateComponent<WBoxComponent>();
	SetRootComponent(mBoxComp);
	if (auto BoxComp = mBoxComp.lock())
	{
		BoxComp->ActivatePhysicBody();
		BoxComp->SetExtent(XMFLOAT3(50, 0.5f, 50));
		BoxComp->SetMotionType(EMotionType::Kinematic);
		BoxComp->SetObjectChannel(EObjectChannel::EOC_NoneMoving);
	}

	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	if (auto StaticMeshComp = mStaticMeshComp.lock())
	{
		StaticMeshComp->SetupAttachment(GetRootComponent());
		StaticMeshComp->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_DefaultFloor));
	}
}

void AFloor::BeginPlay()
{
	Super::BeginPlay();
}