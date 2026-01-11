#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRenderer.h>

class FPhysicsDebugRenderer : public JPH::DebugRenderer
{
public:
	// DX Resource 초기화
	FPhysicsDebugRenderer();

	virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;

private:

};