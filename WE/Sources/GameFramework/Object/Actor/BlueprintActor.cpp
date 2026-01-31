#include "BlueprintActor.h"

ABlueprintActor::ABlueprintActor()
{
	mStaticMeshComp = CreateComponent<WStaticMeshComponent>();
	if (auto Comp = mStaticMeshComp.lock())
	{
		Comp->SetupAttachment(GetRootComponent());
		mBlueprintMap["StaticMesh"] = Comp.get();
	}
	

	mBlueprintMap["Value"] = &Value;
}
