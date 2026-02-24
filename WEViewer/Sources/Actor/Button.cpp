#include "Button.h"
#include "Component/BoxComponent.h"
#include "Component/StaticMeshComponent.h"

AButton::AButton()
{
	mBoxComp = CreateComponent<WBoxComponent>();
	SetRootComponent(mBoxComp->GetWeakPtr<WBoxComponent>());

	mBoxComp->ActivatePhysicBody();
	mBoxComp->SetExtent(XMFLOAT3(0.5f, 0.5f, 0.5f));
	mBoxComp->SetMotionType(EMotionType::Kinematic);
	mBoxComp->SetObjectChannel(EObjectChannel::EOC_PhysicsBody);

	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	mStaticMeshComp->SetupAttachment(GetRootComponent());
	mStaticMeshComp->SetStaticMesh("SM_RedBox");
}

void AButton::Interaction()
{
	mOnButtonInteracted.Broadcast();
	std::cout << "Interaction" << std::endl;
}

void AButton::OnBeginInteractionFocus()
{
	mStaticMeshComp->SetStaticMesh("SM_GreenBox"); 
}

void AButton::OnEndInteractionFocus()
{
	mStaticMeshComp->SetStaticMesh("SM_RedBox");
}
