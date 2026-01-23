#include "PhysicsSphere.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "Component/SphereComponent.h"

APhysicsSphere::APhysicsSphere()
{
	mSphereComp = CreateComponent<WSphereComponent>();
	SetRootComponent(mSphereComp);
	if (auto Comp = mSphereComp.lock())
	{
		Comp->SetMotionType(EMotionType::Dynamic);
		Comp->ActivatePhysicBody();
		Comp->SetRadius(0.5f);
	}

	auto Component = CreateComponent<WStaticMeshComponent>().lock();
	Component->SetupAttachment(GetRootComponent());
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_RustedIron2Sphere));
}

void APhysicsSphere::BeginPlay()
{
	Super::BeginPlay();

	mSphereComp.lock()->mOnHitDelegate.Bind(this, &APhysicsSphere::OnHit);
}

void APhysicsSphere::OnHit(TWeakPtr<WPhysicsComponent> Other, XMFLOAT3 ImpactPoint)
{
	Destroy();
}
