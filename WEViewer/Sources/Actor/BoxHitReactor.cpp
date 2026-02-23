#include "BoxHitReactor.h"
#include "Component/BoxComponent.h"
#include "Component/StaticMeshComponent.h"

ABoxHitReactor::ABoxHitReactor()
{
	WBoxComponent* BoxComp = CreateComponent<WBoxComponent>();
	mPhysicsComp = BoxComp;
	SetRootComponent(mPhysicsComp->GetWeakPtr<WBoxComponent>());

	BoxComp->ActivatePhysicBody();
	BoxComp->SetExtent(XMFLOAT3(0.5f, 0.5f, 0.5f));
	BoxComp->SetMotionType(EMotionType::Dynamic);
	BoxComp->SetObjectChannel(EObjectChannel::EOC_PhysicsBody);

	WStaticMeshComponent* SMComp = CreateComponent<WStaticMeshComponent>();
	SMComp->SetupAttachment(GetRootComponent());
	SMComp->SetStaticMesh("SM_ScuffedGoldBox");
}
