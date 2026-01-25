#pragma once
#include <Jolt/Jolt.h>
#include <DirectXMath.h>
#include "HitResult.h"

using namespace DirectX;

namespace Physics
{
	void LineTrace(XMFLOAT3 Start, XMFLOAT3 End, FHitResult& HitResult);
}