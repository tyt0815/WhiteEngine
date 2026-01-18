
#pragma once

#include "ObjectChannel.h"
#include "DirectX/DXMath.h"
#include "Utility/Delegate.h"

using namespace DirectX;

namespace JPH 
{
	class Body;
	class BodyCreationSettings;
}

namespace Physics
{
	void Startup();

	void Cleanup();

	void Tick(float DeltaTime);
}