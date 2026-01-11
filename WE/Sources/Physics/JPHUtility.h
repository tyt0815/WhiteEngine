#pragma once
#include <Jolt/Jolt.h>
#include "DirectX/DXMath.h"

using namespace JPH;

inline XMFLOAT3 ToDXLocation(RVec3 JPHPos)
{
	return XMFLOAT3(JPHPos.GetX(), JPHPos.GetY(), -JPHPos.GetZ());
}

inline RVec3 ToJPHPosition(XMFLOAT3 DXLocation)
{
	return RVec3(DXLocation.x, DXLocation.y, -DXLocation.z);
}

inline XMFLOAT4 ToDXQuatRotation(JPH::Quat JPHQuat)
{
	return XMFLOAT4(-JPHQuat.GetX(), -JPHQuat.GetY(), JPHQuat.GetZ(), JPHQuat.GetW());
}