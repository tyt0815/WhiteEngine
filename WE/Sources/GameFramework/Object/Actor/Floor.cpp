#include "Floor.h"
#include "GameFramework/Object/World/World.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "GameFramework/Object/Component/SplineComponent.h"
#include "Component/BoxComponent.h"

AFloor::AFloor()
{
	constexpr float Height = 0.10f;
	mStaticMeshComp = CreateComponent<WStaticMeshComponent>()->GetWeakPtr<WStaticMeshComponent>();
	SetRootComponent(mStaticMeshComp);
	if (auto StaticMeshComp = mStaticMeshComp.lock())
	{
		StaticMeshComp->SetStaticMesh(FStaticMeshManager::GetStaticMesh("SM_DefaultFloor"));
	}

	mBoxComp = CreateComponent<WBoxComponent>()->GetWeakPtr<WBoxComponent>();
	if (auto BoxComp = mBoxComp.lock())
	{
		BoxComp->SetupAttachment(GetRootComponent());
		BoxComp->ActivatePhysicBody();
		BoxComp->SetExtent(XMFLOAT3(100, Height, 100));
		BoxComp->SetMotionType(EMotionType::Static);
		BoxComp->SetObjectChannel(EObjectChannel::EOC_WorldStatic);
		BoxComp->SetRelativeLocation(XMFLOAT3(0, -Height, 0));
	}
}

void AFloor::BeginPlay()
{
	Super::BeginPlay();
}