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
}