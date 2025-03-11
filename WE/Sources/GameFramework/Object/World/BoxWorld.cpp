#include "BoxWorld.h"
#include "GameFramework/Object/Actor/Box.h"

WBoxWorld::WBoxWorld()
{

}

void WBoxWorld::BuildWorldActors()
{
	AActor* Actor;
	FTransform Transform;
	Transform.Translation.x = 1.0f;
	Actor = SpawnActor<ABox>();
	Actor->SetTransform(Transform);
	Transform.Translation.x = -1.0f;
	Actor = SpawnActor<ABox>();
	Actor->SetTransform(Transform);
}
