#include "BoxWorld.h"
#include "Runtime/Object/Box.h"

WBoxWorld::WBoxWorld()
{

}

void WBoxWorld::BuildWorldActors()
{
	FTransform Transform;
	Transform.Translation.x = 1.0f;
	SpawnActor<ABox>(EActorType::EAT_Opaque, Transform);
	Transform.Translation.x = -1.0f;
	SpawnActor<ABox>(EActorType::EAT_Opaque, Transform);
}
