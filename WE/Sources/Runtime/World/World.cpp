#include "World.h"
#include "Render/GeometryGenerator.h"

WWorld::WWorld()
{
	
}

WWorld::~WWorld()
{
	unique_ptr<WWorld> a;
}

bool WWorld::Initialize()
{
	BuildWorldActors();
	return true;
}

void WWorld::Tick()
{
	// TODO
}
