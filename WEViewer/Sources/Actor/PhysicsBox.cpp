#include "PhysicsBox.h"

APhysicsBox::APhysicsBox()
{
	mBoxComp = CreateComponent<WBoxComponent>()->GetWeakPtr<WBoxComponent>();
	SetRootComponent(mBoxComp);
	if (auto Box = mBoxComp.lock())
	{
		Box->ActivatePhysicBody();
		Box->SetExtent(XMFLOAT3(0.5f, 0.5f, 0.5f));
		Box->SetMotionType(EMotionType::Kinematic);
		Box->SetObjectChannel(EObjectChannel::EOC_PhysicsBody);
	}

	mStaticMeshComp = CreateComponent<WStaticMeshComponent>()->GetWeakPtr<WStaticMeshComponent>();
	if (auto StaticMeshComp = mStaticMeshComp.lock())
	{
		StaticMeshComp->SetupAttachment(GetRootComponent());
		StaticMeshComp->SetStaticMesh(FStaticMeshManager::GetStaticMesh("SM_LaminateFlooringBrownBox"));
	}
}
