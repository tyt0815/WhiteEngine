#include "PhysicsSphere.h"
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
}
