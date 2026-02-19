#include "Enemy.h"
#include "Component/StaticMeshComponent.h"

AEnemy::AEnemy()
{
	WStaticMeshComponent* SMComp = CreateComponent<WStaticMeshComponent>();
	SMComp->SetupAttachment(GetRootComponent());
	SMComp->SetStaticMesh("SM_LaminateFlooringBrownBox");

	AddTag("Enemy");
}
