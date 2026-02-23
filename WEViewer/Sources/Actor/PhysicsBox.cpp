#include "PhysicsBox.h"

APhysicsBox::APhysicsBox()
{
	mBoxComp = CreateComponent<WBoxComponent>();
	SetRootComponent(mBoxComp->GetWeakPtr<WBoxComponent>());

	mBoxComp->ActivatePhysicBody();
	mBoxComp->SetExtent(XMFLOAT3(0.5f, 0.5f, 0.5f));
	mBoxComp->SetMotionType(EMotionType::Dynamic);
	mBoxComp->SetObjectChannel(EObjectChannel::EOC_PhysicsBody);
}

void APhysicsBox::AddImpulse(const XMFLOAT3& Impulse, const XMFLOAT3& Point)
{
	mBoxComp->AddImpulse(Impulse, Point);
}
