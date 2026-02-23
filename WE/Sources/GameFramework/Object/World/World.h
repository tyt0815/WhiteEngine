#pragma once

#include "Pawn/Pawn.h"
#include "Physics/HitResult.h"
#include "GameFramework/RenderItemProxy.h"
#include "GameFramework/PhysicsEventProxy.h"
#include "Utility/Container.h"
#include "Utility/Class.h"

#include <array>
#include <queue>

class WPhysicsComponent;

struct FActorSpawnParameter
{
	FTransform Transform;
	AActor* Owner;
};



class WWorld
{
	struct WTimer
	{
		std::function<bool()> Delegate;
		float Time = 0;        // 타이머 설정 간격 (루프 시 재설정용)
		float ExpiredTime = 0; // 만료될 절대 시간 (현재 시간 + 설정 시간)
		bool bLoop = false;

		bool operator>(const WTimer& Other) const
		{
			return ExpiredTime > Other.ExpiredTime;
		}
	};
public:
	WWorld();

	virtual ~WWorld();

public:
	virtual void BeginPlay();

	virtual void Tick(float Delta);

	TWeakPtr<WCameraComponent> GetPlayerCamera() const;

	void SetPlayer(TWeakPtr<APawn> Player);

	template<typename T>
	T* SpawnActor();

	template<typename T>
	T* SpawnActor(const FActorSpawnParameter& Param);

	template<typename T>
	T* SpawnActorByFactory(const std::string& Name);

	template<typename T>
	T* SpawnActorByFactory(const std::string& Name, const FActorSpawnParameter& Param);

	void DestroyActor(const TSharedPtr<AActor>& Actor);

	void DrawDebugLine(XMFLOAT3 Start, XMFLOAT3 End, XMFLOAT4 Color, float LifeSpan);

	void DrawDebugBox(XMFLOAT3 Center, XMFLOAT3 Extend, XMFLOAT4 Quaternion, XMFLOAT4 Color, float Duration);

	void EnqueueOnBeginOverlapEvent(const FPhysicEventInfo& Info);

	void EnqueueOnHitEvent(const FPhysicEventInfo& Info);

	void ActivateActor(AActor* Actor);

	void DeactivateActor(AActor* Actor);

	void EnqueueTick(WObject* Object);

	void DequeueTick(WObject* Object);

	void EnqueuePhysicsComponent(WPhysicsComponent* PhysicsComp);

	void DequeuePhysicsComponent(WPhysicsComponent* PhysicsComp);

	void AddWorldTimer(float InTime, bool bInLoop, std::function<bool()> InDelegate);

	void LineTrace(
		XMFLOAT3 Start, XMFLOAT3 End,
		const std::vector<AActor*>& ActorsToIgnore,
		FHitResult& HitResult,
		bool bDrawDebug = false,
		float DebugDuration = 10
	);

	void LineTrace_Internal(
		XMFLOAT3 Start, XMFLOAT3 End,
		const std::vector<JPH::BodyID>& BodiesToIgnore,
		FHitResult& HitResult,
		bool bDrawDebug = false,
		float DebugDuration = 10
	);

	void LineTraceByObjectChannel(
		XMFLOAT3 Start, XMFLOAT3 End,
		const std::vector<AActor*>& ActorsToIgnore,
		const std::vector<JPH::ObjectLayer> ObjectChannels,
		FHitResult& HitResult,
		bool bDrawDebug = false,
		float DebugDuration = 10
	);

	void BoxTrace(
		XMFLOAT3 Start, XMFLOAT3 End,
		XMFLOAT3 Extend, XMFLOAT3 Rotation,
		const std::vector<AActor*>& ActorsToIgnore, FHitResult& HitResult,
		bool bDrawDebug = false, float DebugDuration = 10
	);

	void BoxTrace_Internal(
		XMFLOAT3 Start, XMFLOAT3 End,
		XMFLOAT3 Extend, XMFLOAT3 Rotation,
		const std::vector<JPH::BodyID>& BodiesToIgnore,
		FHitResult& HitResult,
		bool bDrawDebug = false, float DebugDuration = 10
	);

	void BoxTraceByObjectChannel(
		XMFLOAT3 Start, XMFLOAT3 End,
		XMFLOAT3 Extend, XMFLOAT3 Rotation,
		const std::vector<AActor*>& ActorsToIgnore,
		const std::vector<JPH::ObjectLayer>& ObjectChannels,
		FHitResult& HitResult,
		bool bDrawDebug = false, float DebugDuration = 10
	);

	void CapsuleTrace_Internal(XMFLOAT3 Start, XMFLOAT3 End, float Radius, float HalfHeight, XMFLOAT3 Rotation,
		const std::vector<JPH::BodyID>& BodiesToIgnore, FHitResult& HitResult, bool bDrawDebug, float DebugDuration);

	void CapsuleTrace(XMFLOAT3 Start, XMFLOAT3 End, float Radius, float HalfHeight, XMFLOAT3 Rotation,
		const std::vector<AActor*>& ActorsToIgnore, FHitResult& HitResult, bool bDrawDebug, float DebugDuration);

	void CapsuleTraceByObjectChannel(XMFLOAT3 Start, XMFLOAT3 End, float Radius, float HalfHeight, XMFLOAT3 Rotation,
		const std::vector<AActor*>& ActorsToIgnore, const std::vector<JPH::ObjectLayer>& ObjectChannels,
		FHitResult& HitResult, bool bDrawDebug, float DebugDuration);

	void SphereTrace(XMFLOAT3 Start, XMFLOAT3 End, float Radius,
		const std::vector<AActor*>& ActorsToIgnore, FHitResult& HitResult, bool bDrawDebug, float DebugDuration);

	void SphereTrace_Internal(XMFLOAT3 Start, XMFLOAT3 End, float Radius,
		const std::vector<JPH::BodyID>& BodiesToIgnore, FHitResult& HitResult, bool bDrawDebug, float DebugDuration);

	void SphereTraceByObjectChannel(XMFLOAT3 Start, XMFLOAT3 End, float Radius,
		const std::vector<AActor*>& ActorsToIgnore, const std::vector<JPH::ObjectLayer>& ObjectChannels,
		FHitResult& HitResult, bool bDrawDebug, float DebugDuration);

	void SphereOverlap(
		XMFLOAT3 Location,
		float Radius,
		const std::vector<AActor*>& ActorsToIgnore,
		TArray<FHitResult>& HitResults,
		bool bDrawDebug,
		float DebugDuration = 10
	);

private:
	void ExtractActorsPhysicsBodyID(const std::vector<AActor*>& Actors, std::vector<JPH::BodyID>& Bodies);

	void AfterLineTrace(XMFLOAT3 Start, XMFLOAT3 End, const FHitResult& HitResult, bool bDrawDebug, float DebugDuration);

	void AfterBoxTrace(XMFLOAT3 Start, XMFLOAT3 End, XMFLOAT3 Extend, XMFLOAT4 Quaternion, const FHitResult& HitResult, bool bDrawDebug, float DebugDuration);

	void AfterCapsuleTrace(XMFLOAT3 Start, XMFLOAT3 End, float Radius, float HalfHeight, XMFLOAT4 Quaternion, const FHitResult& HitResult, bool bDrawDebug, float DebugDuration);

	void AfterSphereTrace(XMFLOAT3 Start, XMFLOAT3 End, float Radius, const FHitResult& HitResult, bool bDrawDebug, float DebugDuration);

	void AfterSphereOverlap(XMFLOAT3 Location, float Radius, const TArray<FHitResult>& HitResults, bool bDrawDebug, float DebugDuration);

	void FlushDestroyQueue();

	void TickWorldTimer(float DeltaSecond);

	template<typename T>
	void RegisterActor(TSharedPtr<T>& Actor);

	void OnSpawnActor(AActor* Actor, const FActorSpawnParameter& Param);

	TArray<TSharedPtr<AActor>> mAllActors;

	TArray<AActor*> mActiveActorQueue;

	std::vector<FPhysicEventInfo> mOnBeginOverlapEventQueue;

	std::vector<FPhysicEventInfo> mOnHitEventQueue;

	std::vector<TSharedPtr<AActor>> DestroyQueue;

	std::array<std::array<std::vector<WObject*>, static_cast<int>(ETickPriority::ETP_None)>, static_cast<int>(ETickGroup::ETG_None)> mTickGroups;

	std::priority_queue<WTimer, std::vector<WTimer>, std::greater<WTimer>> mTimers;

	float mTotalTime = 0;

	__forceinline std::array<std::vector<WObject*>, static_cast<int>(ETickPriority::ETP_None)>& GetTickGroups(ETickGroup TickGroup)
	{
		return mTickGroups[static_cast<int>(TickGroup)];
	}

	__forceinline std::vector<WObject*>& GetTickGroup(ETickGroup TickGroup, ETickPriority Priority)
	{
		return GetTickGroups(TickGroup)[static_cast<int>(Priority)];
	}

	std::vector<WPhysicsComponent*> mPhysicsComponentQueue;

	TWeakPtr<APawn> mPlayer;

	std::mutex mEventQueueMutex;

	struct FProfilingData
	{
		double Total()
		{
			return Time_Tick_PrePhysics + Time_Update_Physics + Time_Physics_Event +
				Time_Tick_PostPhysics + Time_Flush_DestroyQueue + Time_Update_Render_Items;
		}

		double Time_Tick_PrePhysics;
		double Time_Update_Physics;
		double Time_Physics_Event;
		double Time_Tick_PostPhysics;
		double Time_Flush_DestroyQueue;
		double Time_Update_Render_Items;
	};
	void UpdateProfilingData(float DeltaSecond, const FProfilingData& Data);

	FProfilingData mProfilingData;

	FProfilingData mMaxProfilingData;

public:
	FRenderItemProxy mRenderItemProxy;

	std::mutex mRenderItemProxyMetex;

	inline TWeakPtr<APawn> GetPlayer() const
	{
		return mPlayer;
	}

	inline FMeshCBProxy* GetMeshCBProxy(size_t i)
	{
		return mRenderItemProxy.GetMeshCBProxy(i);
	}

	inline FSubmeshCBProxy* GetSubmeshCBProxy(size_t i)
	{
		return mRenderItemProxy.GetSubmeshCBProxy(i);
	}

	inline FStaticMeshProxy* GetStaticMeshProxy(size_t i)
	{
		return mRenderItemProxy.GetStaticMeshProxy(i);
	}

	inline FDirectionalLightProxy* GetDirectionalLightProxy(size_t i)
	{
		return mRenderItemProxy.GetDirectionalLightProxy(i);
	}

	inline size_t AllocateMeshCbProxy()
	{
		return mRenderItemProxy.AllocateMeshCbProxy();
	}

	inline size_t AllocateSubmeshCbProxy()
	{
		return mRenderItemProxy.AllocateSubmeshCbProxy();
	}

	inline size_t AllocateStaticMeshCbProxy()
	{
		return mRenderItemProxy.AllocateStaticMeshCbProxy();
	}

	inline size_t AllocateDirectionalLightCbProxy()
	{
		return mRenderItemProxy.AllocateDirectionalLightCbProxy();
	}
};

extern WWorld* g_World;

inline WWorld* GetWorld()
{
	return g_World;
}

template<typename T>
inline T* WWorld::SpawnActor()
{
	FActorSpawnParameter Param;
	return SpawnActor<T>(Param);
}

template<typename T>
inline T* WWorld::SpawnActor(const FActorSpawnParameter& Param)
{
	TSharedPtr<T> Actor = MakeShared<T>();
	RegisterActor(Actor);

	OnSpawnActor(Actor.get(), Param);

	return Actor.get();
}

template<typename T>
inline T* WWorld::SpawnActorByFactory(const std::string& Name)
{
	FActorSpawnParameter Param;

	return SpawnActorByFactory<T>(Name, Param);
}

template<typename T>
inline T* WWorld::SpawnActorByFactory(const std::string& Name, const FActorSpawnParameter& Param)
{
	TSharedPtr<T> Actor = FActorFactory::CreateActor<T>(Name);
	assert(Actor != nullptr && "부모 클래스를 제대로 설정했는지 확인");
	RegisterActor<T>(Actor);

	OnSpawnActor(Actor.get(), Param);

	return Actor.get();
}

template<typename T>
inline void WWorld::RegisterActor(TSharedPtr<T>& Actor)
{
	int ActorId = (int)mAllActors.size();
	mAllActors.emplace_back(Actor);
	Actor->mActorId = ActorId;
}
