#pragma once
#ifndef JPH_DEBUG_RENDERER
#define JPH_DEBUG_RENDERER
#endif
#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRenderer.h>

#include <DirectXMath.h>
#include <vector>
#include "Utility/Memory.h"

using namespace JPH;
using namespace DirectX;

class FPhysicsDebugRenderer : public JPH::DebugRenderer
{
	class FPhysicsTriangleBatch : public JPH::RefTargetVirtual
	{
	public:
		struct Triangle 
		{
			Vertex v1, v2, v3;
		};
		std::vector<Triangle> mTriangles;

		// RefTargetVirtual 인터페이스 구현
		virtual void AddRef() override { mRefCount++; }
		virtual void Release() override { if (--mRefCount == 0) delete this; }
	private:
		std::atomic<uint32_t> mRefCount = 0;
	};
public:
	// DX Resource 초기화
	FPhysicsDebugRenderer();

	virtual void DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor) override;
	virtual void DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor, ECastShadow inCastShadow = ECastShadow::Off) override;
	virtual Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override;
	virtual Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const uint32* inIndices, int inIndexCount) override;
	virtual void DrawGeometry(RMat44Arg inModelMatrix, const AABox& inWorldSpaceBounds, float inLODScaleSq, ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) override;
	virtual void DrawText3D(RVec3Arg inPosition, const string_view& inString, ColorArg inColor, float inHeight) override;

public:
	void Clear();

private:
	struct FLine
	{
		XMFLOAT3 Start;
		XMFLOAT3 End;
		XMFLOAT4 Color;
	};
	std::vector<FLine> mLines;

public:
	const std::vector<FLine>& GetLinesView() const
	{
		return mLines;
	}
};

namespace Physics
{
	extern TUniquePtr<FPhysicsDebugRenderer> g_DebugRenderer;
}