#include "World.h"
#include "Render/GeometryGenerator.h"

WWorld::WWorld()
{
	Actors.resize((int)EActorType::EAT_None);
}

WWorld::~WWorld()
{
}

bool WWorld::Initialize()
{
	BuildWorldActors();
	return true;
}

void WWorld::Tick(float Delta)
{
	// TODO

	for (auto& Actor : AllActors) Actor->Tick(Delta);
}
