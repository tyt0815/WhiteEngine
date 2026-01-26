#include "PhysicsBox.h"

APhysicsBox::APhysicsBox()
{
	mBoxComp = CreateComponent<WBoxComponent>();
	SetRootComponent(mBoxComp);
	if (auto Box = mBoxComp.lock())
	{
		Box->ActivatePhysicBody();
		Box->SetExtent(XMFLOAT3(0.5f, 0.5f, 0.5f));
		Box->SetMotionType(EMotionType::Kinematic);
		Box->SetObjectChannel(EObjectChannel::EOC_Moving);
	}

	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	if (auto StaticMeshComp = mStaticMeshComp.lock())
	{
		StaticMeshComp->SetupAttachment(GetRootComponent());
		StaticMeshComp->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_LaminateFlooringBrownBox));
	}
}
