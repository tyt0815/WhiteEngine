#include "Alliance.h"
#include "Component/StaticMeshComponent.h"

AAlliance::AAlliance()
{
	WStaticMeshComponent* SMComp = CreateComponent<WStaticMeshComponent>();
	SMComp->SetupAttachment(GetRootComponent());
	SMComp->SetStaticMesh("SM_ScuffedGoldBox");

	AddTag("Alliance");
}
