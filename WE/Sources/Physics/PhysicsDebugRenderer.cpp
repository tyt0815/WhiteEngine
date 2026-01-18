#include "PhysicsDebugRenderer.h"
#include "JPHUtility.h"

using namespace JPH;

namespace Physics
{
	TUniquePtr<FPhysicsDebugRenderer> g_DebugRenderer;
}

FPhysicsDebugRenderer::FPhysicsDebugRenderer()
{
	DebugRenderer::Initialize();
}

void FPhysicsDebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
	FLine Line;
	Line.Start = ToDXLocation(inFrom);
	Line.End = ToDXLocation(inTo);
	Line.Color = { inColor.r / 255.f, inColor.g / 255.f, inColor.b / 255.f, inColor.a / 255.f };

	mLines.push_back(Line);
}

void FPhysicsDebugRenderer::DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor, ECastShadow inCastShadow)
{
	DrawLine(inV1, inV2, inColor);
	DrawLine(inV2, inV3, inColor);
	DrawLine(inV3, inV1, inColor);
}

FPhysicsDebugRenderer::Batch FPhysicsDebugRenderer::CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount)
{
	return Batch();
}

// DrawShape = true시 호출되는 곳
FPhysicsDebugRenderer::Batch FPhysicsDebugRenderer::CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const uint32* inIndices, int inIndexCount)
{
	FPhysicsTriangleBatch* batch = new FPhysicsTriangleBatch();

	// 인덱스를 따라가며 삼각형 데이터 생성
	for (int i = 0; i < inIndexCount; i += 3)
	{
		FPhysicsTriangleBatch::Triangle tri;
		tri.v1 = inVertices[inIndices[i]];
		tri.v2 = inVertices[inIndices[i + 1]];
		tri.v3 = inVertices[inIndices[i + 2]];
		batch->mTriangles.push_back(tri);
	}

	return batch; // Jolt의 Ref<Batch>가 소유권을 가집니다.
}

// DrawShape = true시 호출되는 곳
// CreateTriangleBatch다음 호출됨
void FPhysicsDebugRenderer::DrawGeometry(RMat44Arg inModelMatrix, const AABox& inWorldSpaceBounds, float inLODScaleSq, ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode)
{
	// 1. 넘겨받은 Geometry에서 Batch를 꺼내옴
	// Jolt의 LOD 시스템 등에 따라 여러 개일 수 있으나 보통 하나입니다.
	for (const JPH::DebugRenderer::LOD& lod : inGeometry->mLODs)
	{
		FPhysicsTriangleBatch* myBatch = static_cast<FPhysicsTriangleBatch*>(lod.mTriangleBatch.GetPtr());

		if (!myBatch) continue;

		// 2. 각 삼각형을 월드 좌표로 변환하여 DrawLine으로 보냄
		for (const auto& tri : myBatch->mTriangles)
		{
			// 모델 행렬을 적용하여 월드 좌표 계산
			RVec3 v1 = inModelMatrix * Vec3(tri.v1.mPosition);
			RVec3 v2 = inModelMatrix * Vec3(tri.v2.mPosition);
			RVec3 v3 = inModelMatrix * Vec3(tri.v3.mPosition);

			// 와이어프레임으로 그리기 위해 세 변을 DrawLine 호출
			DrawLine(v1, v2, inModelColor);
			DrawLine(v2, v3, inModelColor);
			DrawLine(v3, v1, inModelColor);
		}
	}
}

void FPhysicsDebugRenderer::DrawText3D(RVec3Arg inPosition, const string_view& inString, ColorArg inColor, float inHeight)
{
}

void FPhysicsDebugRenderer::Clear()
{
	mLines.clear();
}
