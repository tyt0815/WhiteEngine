#include "World.h"
#include "Render/GeometryGenerator.h"

WWorld* gWorld = nullptr;

WWorld::WWorld()
{
	if (gWorld)
	{
		throw L"이미 월드가 하나 생성되었습니다.";
	}
	gWorld = this;
	Actors.resize((int)EActorType::EAT_None);
}

WWorld::~WWorld()
{
	gWorld = nullptr;
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
