#include "PhysicsSphere.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "Component/SphereComponent.h"

APhysicsSphere::APhysicsSphere()
{
	mSphereComp = CreateComponent<WSphereComponent>();
	SetRootComponent(mSphereComp);
	mSphereComp->SetObjectType(EObjectType::EOT_Dynamic);
	mSphereComp->ActivatePhysicBody();
	mSphereComp->SetRadius(0.5f);

	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	Component->SetupAttachment(GetRootComponent());
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_RustedIron2Sphere));
}

void APhysicsSphere::Tick_PrePhysics(float DeltaTiem)
{
	Super::Tick_PrePhysics(DeltaTiem);
}
