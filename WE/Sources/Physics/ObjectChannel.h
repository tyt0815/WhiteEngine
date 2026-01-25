#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

// Layer that objects can be in, determines which other objects it can collide with
// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
// but only if you do collision testing).
namespace EObjectChannel
{
	enum EObjectChannel : JPH::ObjectLayer
	{
		EOC_NoneMoving = 0,
		EOC_Moving,
		EOC_None
	};
};