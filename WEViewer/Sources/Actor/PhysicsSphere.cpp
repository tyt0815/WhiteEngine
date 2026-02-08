#include "PhysicsSphere.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "Component/SphereComponent.h"

APhysicsSphere::APhysicsSphere()
{
	mSphereComp = CreateComponent<WSphereComponent>()->GetWeakPtr<WSphereComponent>();
	SetRootComponent(mSphereComp);
	if (auto Comp = mSphereComp.lock())
	{
		Comp->SetMotionType(EMotionType::Dynamic);
		Comp->ActivatePhysicBody();
		Comp->SetRadius(0.5f);
	}

	auto Component = CreateComponent<WStaticMeshComponent>();
	Component->SetupAttachment(GetRootComponent());
	Component->SetStaticMesh(FStaticMeshManager::GetStaticMesh("SM_RustedIron2Sphere"));
}
