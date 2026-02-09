#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <DirectXMath.h>
#include "HitResult.h"

using namespace DirectX;

namespace Physics
{
	void LineTrace(
		XMFLOAT3 Start, XMFLOAT3 End,
		FHitResult& HitResult,
		const TArray<JPH::BodyID>& BodiesToIgnore
	);

	void LineTrace(
		XMFLOAT3 Start, XMFLOAT3 End,
		FHitResult& HitResult,
		const TArray<JPH::ObjectLayer>& InObjectLayers,
		const TArray<JPH::BodyID>& BodiesToIgnore
	);

	void LineTrace(
		XMFLOAT3 Start, XMFLOAT3 End,
		FHitResult& HitResult,
		const JPH::BroadPhaseLayerFilter& inBroadPhaseLayerFilter = { },
		const JPH::ObjectLayerFilter& inObjectLayerFilter = { },
		const JPH::BodyFilter& inBodyFilter = { }
	);

	void ShapeTrace(
		JPH::ShapeRefC InShape,
		JPH::RMat44 InMat,
		JPH::RVec3 InDirection,
		FHitResult& HitResult,
		const JPH::BroadPhaseLayerFilter& inBroadPhaseLayerFilter,
		const JPH::ObjectLayerFilter& inObjectLayerFilter,
		const JPH::BodyFilter& inBodyFilter
	);

	void BoxTrace(
		XMFLOAT3 Start, XMFLOAT3 End,
		XMFLOAT3 Extend,
		XMFLOAT4 Quaternion,
		FHitResult& HitResult,
		const JPH::BroadPhaseLayerFilter& inBroadPhaseLayerFilter = { },
		const JPH::ObjectLayerFilter& inObjectLayerFilter = { },
		const JPH::BodyFilter& inBodyFilter = { }
	);

	void BoxTrace(
		XMFLOAT3 Start, XMFLOAT3 End,
		XMFLOAT3 Extend,
		XMFLOAT4 Quaternion,
		FHitResult& HitResult,
		const TArray<JPH::BodyID>& BodiesToIgnore
	);

	void BoxTrace(
		XMFLOAT3 Start, XMFLOAT3 End,
		XMFLOAT3 Extend,
		XMFLOAT4 Quaternion,
		FHitResult& HitResult,
		const TArray<JPH::ObjectLayer>& InObjectLayers,
		const TArray<JPH::BodyID>& BodiesToIgnore
	);

	void CapsuleTrace(
		XMFLOAT3 Start,
		XMFLOAT3 End,
		float Radius,
		float HalfHeight,
		XMFLOAT4 Quaternion,
		FHitResult& HitResult,
		const JPH::BroadPhaseLayerFilter& inBroadPhaseLayerFilter,
		const JPH::ObjectLayerFilter& inObjectLayerFilter,
		const JPH::BodyFilter& inBodyFilter
	);

	void CapsuleTrace(XMFLOAT3 Start, XMFLOAT3 End, float Radius, float HalfHeight, XMFLOAT4 Quaternion, FHitResult& HitResult, const TArray<JPH::BodyID>& BodiesToIgnore);

	void CapsuleTrace(XMFLOAT3 Start, XMFLOAT3 End, float Radius, float HalfHeight, XMFLOAT4 Quaternion, FHitResult& HitResult, const TArray<JPH::ObjectLayer>& InObjectLayers, const TArray<JPH::BodyID>& BodiesToIgnore);

	void SphereTrace(
		XMFLOAT3 Start,
		XMFLOAT3 End,
		float Radius,
		FHitResult& HitResult,
		const JPH::BroadPhaseLayerFilter& inBroadPhaseLayerFilter,
		const JPH::ObjectLayerFilter& inObjectLayerFilter,
		const JPH::BodyFilter& inBodyFilter
	);

	void SphereTrace(XMFLOAT3 Start, XMFLOAT3 End, float Radius, FHitResult& HitResult, const TArray<JPH::BodyID>& BodiesToIgnore);

	void SphereTrace(XMFLOAT3 Start, XMFLOAT3 End, float Radius, FHitResult& HitResult, const TArray<JPH::ObjectLayer>& InObjectLayers, const TArray<JPH::BodyID>& BodiesToIgnore);

	void ShapeOverlap(
		JPH::ShapeRefC InShape,
		JPH::RMat44 InMat, // 현재 제자리의 위치/회전 행렬
		TArray<FHitResult>& HitResults,
		const JPH::BroadPhaseLayerFilter& inBPFilter,
		const JPH::ObjectLayerFilter& inObjFilter,
		const JPH::BodyFilter& inBodyFilter
	);

	void SphereOverlap(
		XMFLOAT3 Location,
		float Radius,
		TArray<FHitResult>& HitResults,
		const JPH::BroadPhaseLayerFilter& inBroadPhaseLayerFilter = { },
		const JPH::ObjectLayerFilter& inObjectLayerFilter = { },
		const JPH::BodyFilter& inBodyFilter = { }
	);

	void SphereOverlap(
		XMFLOAT3 Location,
		float Radius,
		TArray<FHitResult>& HitResults,
		const TArray<JPH::BodyID>& BodiesToIgnore
	);

	void SphereOverlap(
		XMFLOAT3 Location,
		float Radius,
		TArray<FHitResult>& HitResults,
		const TArray<JPH::ObjectLayer>& InObjectLayers,
		const TArray<JPH::BodyID>& BodiesToIgnore
	);
}