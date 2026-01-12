#include "Cylinder.h"
#include "GameFramework/Object/Component/StaticMeshComponent.h"

ACylinder::ACylinder()
{
	WSceneComponent* DummyRoot = CreateComponent<WSceneComponent>();
	SetRootComponent(DummyRoot);
	WStaticMeshComponent* Component = CreateComponent<WStaticMeshComponent>();
	Component->SetupAttachment(DummyRoot);
	Component->SetStaticMesh(GetStaticMeshManager()->GetStaticMesh(ESMT_RustedIron2Cylinder));
	Component->SetLocalRotation(XMFLOAT3(90, 0, 0));
}
