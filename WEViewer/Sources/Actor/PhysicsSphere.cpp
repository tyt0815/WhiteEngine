#include "PhysicsSphere.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"
#include "Component/SphereComponent.h"

APhysicsSphere::APhysicsSphere()
{
	mSphereComp = CreateComponent<WSphereComponent>();
	SetRootComponent(mSphereComp);
	mSphereComp->SetMotionType(EMotionType::Dynamic);
	mSphereComp->ActivatePhysicBody();
	mSphereComp->SetRadius(0.5f);

	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	Component->SetupAttachment(GetRootComponent());
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_RustedIron2Sphere));
}

void APhysicsSphere::BeginPlay()
{
	Super::BeginPlay();

	mSphereComp->mOnHitDelegate.Bind(this, &APhysicsSphere::OnHit);
}

void APhysicsSphere::Tick_PrePhysics(float DeltaTiem)
{
	Super::Tick_PrePhysics(DeltaTiem);
}

void APhysicsSphere::OnHit(WPrimitiveComponent* Other)
{
	Destroy();
}
