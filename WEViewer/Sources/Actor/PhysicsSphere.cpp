#include "PhysicsSphere.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

APhysicsSphere::APhysicsSphere()
{
	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	SetRootComponent(Component);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_RustedIron2Sphere));

	mBody = CreateSphereBody(0.5f, EObjectType::EOT_Dynamic);
	mBody->AddBody();
}

void APhysicsSphere::Tick(float DeltaTiem)
{
	Super::Tick(DeltaTiem);

	 SetActorTransform(mBody->GetTransform());

	 static float Time = 0;
	 Time += DeltaTiem;
	 if (Time > 1)
	 {
		 Time = 0;
		 mBody->SetPosition(XMFLOAT3(0.0f, 0.0f, 5.0f));
	 }
}
