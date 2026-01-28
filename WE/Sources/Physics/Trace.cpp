#include "Trace.h"
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include "JPHUtility.h"

using namespace JPH;

class FTraceBroadPhaseLayerFilter : public BroadPhaseLayerFilter
{
public:
	FTraceBroadPhaseLayerFilter(const TArray<BroadPhaseLayer>& Layers) :
		mBroadPhaseLayers(Layers)
	{

	}

	virtual bool ShouldCollide(BroadPhaseLayer InLayer) const override
	{
		for (BroadPhaseLayer Layer : mBroadPhaseLayers)
		{
			if (InLayer == Layer)
			{
				return true;
			}
		}
		return false;
	}

	__forceinline void AddBroadPhaseLayer(BroadPhaseLayer Layer)
	{
		mBroadPhaseLayers.push_back(Layer);
	}

	__forceinline void AddBroadPhaseLayers(const TArray<BroadPhaseLayer>& Layers)
	{
		mBroadPhaseLayers.insert(mBroadPhaseLayers.end(), Layers.begin(), Layers.end());
	}

private:
	TArray<BroadPhaseLayer> mBroadPhaseLayers;
};

class FTraceObjectLayerFilter : public ObjectLayerFilter
{
public:
	FTraceObjectLayerFilter(const TArray<ObjectLayer>& Layers) :
		mObjectLayers(Layers)
	{

	}

	virtual bool ShouldCollide(ObjectLayer InLayer) const override
	{
		for (ObjectLayer Layer : mObjectLayers)
		{
			if (InLayer == Layer)
			{
				return true;
			}
		}

		return false;
	}

	__forceinline void AddObjectLayer(ObjectLayer Layer)
	{
		mObjectLayers.push_back(Layer);
	}

	__forceinline void AddObjectLayers(const TArray<ObjectLayer>& Layers)
	{
		mObjectLayers.insert(mObjectLayers.end(), Layers.begin(), Layers.end());
	}

private:
	TArray<ObjectLayer> mObjectLayers;
};

class FTraceBodyFilter : public BodyFilter
{
public:
	FTraceBodyFilter(const TArray<BodyID>& IDs) :
		mIgnoreBodies(IDs)
	{

	}

	// 특정 바디를 무시 리스트에 추가 (예: 시전자 본인)
	virtual bool ShouldCollide(const BodyID& InBodyID) const override
	{
		for (const BodyID& ID : mIgnoreBodies)
		{
			if (InBodyID == ID)
			{
				return false; // 무시 리스트에 있으면 충돌 안 함
			}
		}
		return true;
	}

	__forceinline void AddIgnoreBody(const BodyID& ID)
	{
		mIgnoreBodies.push_back(ID);
	}

	__forceinline void AddIgnoreBodies(const TArray<BodyID>& IDs)
	{
		mIgnoreBodies.insert(mIgnoreBodies.end(), IDs.begin(), IDs.end());
	}

private:
	TArray<BodyID> mIgnoreBodies;
};

namespace Physics
{
	const NarrowPhaseQuery& GetNarrowPhaseQuery();

	BodyInterface& GetBodyInterface();

	const BodyLockInterface& GetBodyLockInterface();

	void LineTrace(
		XMFLOAT3 Start,
		XMFLOAT3 End,
		FHitResult& HitResult,
		const TArray<JPH::BodyID>& BodiesToIgnore
	)
	{
		FTraceBodyFilter BFilter(BodiesToIgnore);

		LineTrace(Start, End, HitResult, {}, {}, BFilter);
	}

	void LineTrace(
		XMFLOAT3 Start, XMFLOAT3 End,
		FHitResult& HitResult,
		const TArray<ObjectLayer>& InObjectLayers,
		const TArray<BodyID>& BodiesToIgnore
	)
	{
		FTraceObjectLayerFilter OLFilter(InObjectLayers);
		FTraceBodyFilter BFilter(BodiesToIgnore);

		LineTrace(Start, End, HitResult, {}, OLFilter, BFilter);
	}

	void LineTrace(
		XMFLOAT3 Start, XMFLOAT3 End,
		FHitResult& HitResult,
		const BroadPhaseLayerFilter& inBroadPhaseLayerFilter,
		const ObjectLayerFilter& inObjectLayerFilter,
		const BodyFilter& inBodyFilter
	)
	{
		HitResult = {};
		RVec3 Origin = ToJPHPosition(Start);
		RVec3 Direction = ToJPHPosition(End) - Origin;
		RRayCast Ray{ Origin, Direction };

		// 단일 타격만 원할 경우:
		RayCastResult Result;

		// 3. 쿼리 실행 (NarrowPhaseQuery 사용)
		const NarrowPhaseQuery& Query = GetNarrowPhaseQuery();
		if (Query.CastRay(Ray, Result, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter))
		{
			// result.mBodyID를 통해 충돌한 바디 정보를 가져옴
			BodyLockRead Lock(GetBodyLockInterface(), Result.mBodyID);
			if (Lock.Succeeded())
			{
				const Body& HitBody = Lock.GetBody();
				HitResult.HitComponent = *reinterpret_cast<TWeakPtr<WPhysicsComponent>*>(HitBody.GetUserData());

				RVec3 ImpactPoint = Direction * Result.mFraction;
				ImpactPoint += Origin;
				HitResult.ImpactPoint = ToDXLocation(ImpactPoint);
			}
		}
	}
}