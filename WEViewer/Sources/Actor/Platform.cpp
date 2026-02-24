#include "Platform.h"
#include "Component/StaticMeshComponent.h"
#include "Component/BoxComponent.h"

APlatform::APlatform()
{
	mBoxComp = CreateComponent<WBoxComponent>();
	SetRootComponent(mBoxComp->GetWeakPtr<WSceneComponent>());
	mBoxComp->ActivatePhysicBody();
	mBoxComp->SetExtent(XMFLOAT3(0.5f, 0.05f, 0.5f));
	mBoxComp->SetMotionType(EMotionType::Static);
	mBoxComp->SetObjectChannel(EObjectChannel::EOC_PhysicsBody);

	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	mStaticMeshComp->SetupAttachment(mBoxComp);
	mStaticMeshComp->SetStaticMesh("SM_WhiteBox");
	mStaticMeshComp->SetRelativeScale(XMFLOAT3(1, 0.1f, 1));
}
