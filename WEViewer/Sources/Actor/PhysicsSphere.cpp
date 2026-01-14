#include "PhysicsSphere.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

APhysicsSphere::APhysicsSphere()
{
	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_RustedIron2Sphere));

	//mbPhysicSimulate = true;
	//mActorPhysicsShape = EPhysicsShape::EPS_Sphere;
	//mSpherePhysicsRadius = 0.5f;
}

void APhysicsSphere::Tick_PrePhysics(float DeltaTiem)
{
	Super::Tick_PrePhysics(DeltaTiem);
}
