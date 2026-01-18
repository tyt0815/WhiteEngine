#include "TestWorld.h"
#include "../Actor/PhysicsSphere.h"

WTestWorld::WTestWorld()
{
	auto PSphere = SpawnActor<APhysicsSphere>().lock();
	PSphere->SetActorLocation(XMFLOAT3(0.0f, 5.0f, 5.0f));
}
