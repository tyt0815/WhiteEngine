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

	AddTag("WorldStatic");
}

void AButton::Interaction()
{
	mOnButtonInteracted.Broadcast();
}

void AButton::OnBeginInteractionFocus()
{
	mStaticMeshComp->SetStaticMesh("SM_GreenBox"); 
}

void AButton::OnEndInteractionFocus()
{
	mStaticMeshComp->SetStaticMesh("SM_RedBox");
}

void AButton::OnHit(WSceneComponent* Instigator, WPhysicsComponent* HittedComponent, XMFLOAT3 ImpulseDir, XMFLOAT3 ImpactPoint, XMFLOAT3 Normal, float Distance, float Damage)
{
	Interaction();
}
