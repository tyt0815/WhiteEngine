#include "BoxWorld.h"
#include "Application/TestApplication/Actor/Box.h"

WBoxWorld::WBoxWorld()
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
