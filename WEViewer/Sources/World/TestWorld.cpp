#include "TestWorld.h"
#include "../Actor/PhysicsSphere.h"

WTestWorld::WTestWorld()
{
	APhysicsSphere* PSphere = SpawnActor<APhysicsSphere>();
	PSphere->SetActorLocation(XMFLOAT3(0.0f, 5.0f, 5.0f));
}
