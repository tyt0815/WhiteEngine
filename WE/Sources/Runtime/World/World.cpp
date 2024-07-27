#include "World.h"
#include "Render/GeometryGenerator.h"

WWorld::WWorld()
{
	
}

WWorld::~WWorld()
{
	
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
