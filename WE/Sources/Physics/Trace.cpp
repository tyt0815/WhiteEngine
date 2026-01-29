#include "Trace.h"
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include "PhysicsDebugRenderer.h"
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
		RayCastResult InResult;

		// 3. 쿼리 실행 (NarrowPhaseQuery 사용)
		const NarrowPhaseQuery& Query = GetNarrowPhaseQuery();
		if (Query.CastRay(Ray, InResult, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter))
		{
			BodyLockRead Lock(GetBodyLockInterface(), InResult.mBodyID);
			if (Lock.Succeeded())
			{
				const Body& HitBody = Lock.GetBody();
				HitResult.SetActorAndHitComponent(*reinterpret_cast<TWeakPtr<WPhysicsComponent>*>(HitBody.GetUserData()));

				// 1. ImpactPoint 계산 (이미 하신 부분)
				RVec3 ImpactPoint = Ray.mOrigin + InResult.mFraction * Ray.mDirection;
				HitResult.ImpactPoint = ToDXLocation(ImpactPoint);

				// 2. Normal(법선) 계산 ★ 핵심 부분
				// GetWorldSpaceSurfaceNormal 함수를 사용합니다.
				Vec3 Normal = HitBody.GetWorldSpaceSurfaceNormal(InResult.mSubShapeID2, ImpactPoint);

				// 3. HitResult에 저장 (XMFLOAT3로 변환 필요)
				HitResult.Normal = ToDXLocation(Normal);

				HitResult.Distance = InResult.mFraction;
			}
		}
	}

	void ShapeTrace(
		ShapeRefC InShape,
		RMat44 InMat,
		RVec3 InDirection,
		FHitResult& HitResult,
		const BroadPhaseLayerFilter& inBroadPhaseLayerFilter,
		const ObjectLayerFilter& inObjectLayerFilter,
		const BodyFilter& inBodyFilter
	)
	{
		HitResult = {};
		RShapeCast ShapeTrace(
			InShape,
			Vec3::sReplicate(1.0f),
			InMat,
			InDirection
		);

		ShapeCastSettings Settings;
		ClosestHitCollisionCollector<CastShapeCollector> Collector;

		const NarrowPhaseQuery& Query = GetNarrowPhaseQuery();
		Query.CastShape(ShapeTrace, Settings, Vec3::sZero(), Collector, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter);
		if (Collector.HadHit())
		{
			BodyLockRead Lock(GetBodyLockInterface(), Collector.mHit.mBodyID2);
			if (Lock.Succeeded())
			{
				const Body& HitBody = Lock.GetBody();
				HitResult.SetActorAndHitComponent(*reinterpret_cast<TWeakPtr<WPhysicsComponent>*>(HitBody.GetUserData()));

				RVec3 ImpactPoint = Collector.mHit.mContactPointOn2;
				HitResult.ImpactPoint = ToDXLocation(ImpactPoint);

				// 2. Normal(법선) 계산 ★ 핵심 부분
				// GetWorldSpaceSurfaceNormal 함수를 사용합니다.
				Vec3 Normal = HitBody.GetWorldSpaceSurfaceNormal(Collector.mHit.mSubShapeID2, ImpactPoint);

				// 3. HitResult에 저장 (XMFLOAT3로 변환 필요)
				HitResult.Normal = ToDXLocation(Normal);

				HitResult.Distance = Collector.mHit.mFraction;
			}
		}
	}

	void BoxTrace(XMFLOAT3 Start, XMFLOAT3 End, XMFLOAT3 Extend, XMFLOAT4 Quaternion, FHitResult& HitResult, const JPH::BroadPhaseLayerFilter& inBroadPhaseLayerFilter, const JPH::ObjectLayerFilter& inObjectLayerFilter, const JPH::BodyFilter& inBodyFilter)
	{
		// Jolt는 중심에서부터의 거리(HalfExtent)를 사용합니다.
		BoxShapeSettings Settings(RVec3(Extend.x, Extend.y, Extend.z));
		ShapeRefC Box = Settings.Create().Get();

		// 2. 오일러 각(Degree)을 Jolt 쿼터니언으로 변환
		// Degree -> Radian 변환 후 YZX 순서(또는 엔진 기준 순서)로 쿼터니언 생성

		Quat Rot = TOJPHQuatRotation(Quaternion);

		// 3. 시작 지점의 행렬(Matrix) 구성
		// 시작 위치와 회전값을 합쳐서 RMat44를 만듭니다.
		RVec3 JPHStart = ToJPHPosition(Start);
		RVec3 JPHEnd = ToJPHPosition(End);
		RMat44 StartMat = RMat44::sRotationTranslation(Rot, JPHStart);

		// 4. 이동 방향 벡터 계산 (End - Start)
		RVec3 SweepVector = JPHEnd - JPHStart;

		// 5. 앞서 만든 ShapeTrace 호출
		ShapeTrace(
			Box,
			StartMat,
			SweepVector,
			HitResult,
			inBroadPhaseLayerFilter,
			inObjectLayerFilter,
			inBodyFilter
		);
	}

	void BoxTrace(XMFLOAT3 Start, XMFLOAT3 End, XMFLOAT3 Extend, XMFLOAT4 Quaternion, FHitResult& HitResult, const TArray<JPH::BodyID>& BodiesToIgnore)
	{
		FTraceBodyFilter BFilter(BodiesToIgnore);
		BoxTrace(Start, End, Extend, Quaternion, HitResult, {}, {}, BFilter);
	}

	void BoxTrace(XMFLOAT3 Start, XMFLOAT3 End, XMFLOAT3 Extend, XMFLOAT4 Quaternion, FHitResult& HitResult, const TArray<JPH::ObjectLayer>& InObjectLayers, const TArray<JPH::BodyID>& BodiesToIgnore)
	{
		FTraceObjectLayerFilter OLFilter(InObjectLayers);
		FTraceBodyFilter BFilter(BodiesToIgnore);
		BoxTrace(Start, End, Extend, Quaternion, HitResult, {}, OLFilter, BFilter);
	}

	void ShapeOverlap(JPH::ShapeRefC InShape, JPH::RMat44 InMat, TArray<FHitResult>& HitResults, const JPH::BroadPhaseLayerFilter& inBPFilter, const JPH::ObjectLayerFilter& inObjFilter, const JPH::BodyFilter& inBodyFilter)
	{
		HitResults = {};
		// 1. 제자리 충돌 설정
		CollideShapeSettings Settings;
		// 물체가 서로 얼마나 파묻혀야 충돌로 인정할지 (기본값 0.0f)
		Settings.mActiveEdgeMode = EActiveEdgeMode::CollideOnlyWithActive;
		Settings.mBackFaceMode = EBackFaceMode::IgnoreBackFaces;

		// 2. 결과 수집기 (가장 가까운 충돌 하나만 수집)
		AllHitCollisionCollector<CollideShapeCollector> Collector;

		// 3. 제자리 검사 실행 (이동 방향 벡터가 없음)
		const NarrowPhaseQuery& Query = GetNarrowPhaseQuery();
		Query.CollideShape(
			InShape,
			Vec3::sReplicate(1.0f), // Scale
			InMat,                  // 현재 위치/회전
			Settings,
			Vec3::sZero(),          // Base Offset
			Collector,
			inBPFilter,
			inObjFilter,
			inBodyFilter
		);

		if (Collector.HadHit())
		{
			HitResults.resize(Collector.mHits.size());
			for(int i = 0; i < HitResults.size(); ++i)
			{
				BodyLockRead Lock(GetBodyLockInterface(), Collector.mHits[i].mBodyID2);
				if (Lock.Succeeded())
				{
					const Body& HitBody = Lock.GetBody();

					// UserData에서 컴포넌트 복원
					HitResults[i].SetActorAndHitComponent(*reinterpret_cast<TWeakPtr<WPhysicsComponent>*>(HitBody.GetUserData()));

					// CollideShape는 mContactPointOn2를 제공합니다.
					RVec3 ImpactPoint = Collector.mHits[i].mContactPointOn2;
					HitResults[i].ImpactPoint = ToDXLocation(ImpactPoint);

					// 2. Normal(법선) 계산 ★ 핵심 부분
					// GetWorldSpaceSurfaceNormal 함수를 사용합니다.
					Vec3 Normal = HitBody.GetWorldSpaceSurfaceNormal(Collector.mHits[i].mSubShapeID2, ImpactPoint);

					// 3. HitResult에 저장 (XMFLOAT3로 변환 필요)
					HitResults[i].Normal = ToDXLocation(Normal);

					HitResults[i].Distance = Collector.mHits[i].GetEarlyOutFraction();
				}
			}
		}
	}

	void SphereOverlap(
		XMFLOAT3 Location,
		float Radius,
		TArray<FHitResult>& HitResults,
		const JPH::BroadPhaseLayerFilter& inBroadPhaseLayerFilter,
		const JPH::ObjectLayerFilter& inObjectLayerFilter,
		const JPH::BodyFilter& inBodyFilter)
	{
		// 1. 위치 변환 및 구체 모양 생성
		RVec3 JPHPos = ToJPHPosition(Location);
		SphereShapeSettings Settings(Radius);
		ShapeRefC SphereShape = Settings.Create().Get();

		// 2. 구체는 회전이 무의미하므로 단위 행렬에 위치만 더함
		RMat44 CurrentMat = RMat44::sTranslation(JPHPos);

		// 3. 앞서 만든 ShapeOverlap 호출
		// (ShapeOverlap 내부에서 CollideShape를 호출하도록 설계되었으므로 그대로 사용)
		ShapeOverlap(
			SphereShape,
			CurrentMat,
			HitResults,
			inBroadPhaseLayerFilter,
			inObjectLayerFilter,
			inBodyFilter
		);
	}
	void SphereOverlap(XMFLOAT3 Location, float Radius, TArray<FHitResult>& HitResults, const TArray<JPH::BodyID>& BodiesToIgnore)
	{
		FTraceBodyFilter BFilter(BodiesToIgnore);
		SphereOverlap(Location, Radius, HitResults, {}, {}, BFilter);
	}
	void SphereOverlap(XMFLOAT3 Location, float Radius, TArray<FHitResult>& HitResults, const TArray<JPH::ObjectLayer>& InObjectLayers, const TArray<JPH::BodyID>& BodiesToIgnore)
	{
		FTraceObjectLayerFilter OLFilter(InObjectLayers);
		FTraceBodyFilter BFilter(BodiesToIgnore);
		SphereOverlap(Location, Radius, HitResults, {}, OLFilter, BFilter);
	}
}