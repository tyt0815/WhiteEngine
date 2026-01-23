#pragma once

#include "Pawn/Pawn.h"
#include "GameFramework/RenderItemProxy.h"
#include "TickGroup.h"
#include "Utility/Container.h"
#include "Utility/Class.h"

#include <array>

class WPhysicsComponent;

struct FPhysicEventInfo
{
	TWeakPtr<WPhysicsComponent> Comp1;
	TWeakPtr<WPhysicsComponent> Comp2;
	XMFLOAT3 ImpactPoint1;
	XMFLOAT3 ImpactPoint2;
};

class WWorld
{
public:
	WWorld();

	virtual ~WWorld();

public:
	virtual void BeginPlay();

	virtual void Tick(float Delta);

	TWeakPtr<WCameraComponent> GetPlayerCamera() const;

	void SetPlayer(TWeakPtr<APawn> Player);

	template<typename T>
	TWeakPtr<T> SpawnActor();

	void DestroyActor(const TSharedPtr<AActor>& Actor);

	void DrawDebugLine(XMFLOAT3 Start, XMFLOAT3 End, XMFLOAT4 Color, float LifeSpan);

	void EnqueueOnBeginOverlapEvent(const FPhysicEventInfo& Info);

	void EnqueueOnHitEvent(const FPhysicEventInfo& Info);

	void ActivateActor(AActor* Actor);

	void DeactivateActor(AActor* Actor);

	void EnqueueComponentTick(WActorComponent* ActorComp);

	void EnqueueActorTick(AActor* Actor);

	void DequeueComponentTick(WActorComponent* ActorComp);

	void DequeueActorTick(AActor* Actor);

	void EnqueuePhysicsComponent(WPhysicsComponent* PhysicsComp);

	void DequeuePhysicsComponent(WPhysicsComponent* PhysicsComp);

private:
	void FlushDestroyQueue();

	TArray<TSharedPtr<AActor>> mAllActors;

	TArray<AActor*> mActiveActorQueue;

	std::vector<FPhysicEventInfo> mOnBeginOverlapEventQueue;

	std::vector<FPhysicEventInfo> mOnHitEventQueue;

	std::vector<TSharedPtr<AActor>> DestroyQueue;

	std::array<std::vector<AActor*>, ETickGroup::ETG_None> mActorTickGroups;

	std::array<std::vector<WActorComponent*>, ETickGroup::ETG_None> mActorComponentTickGroups;

	std::vector<WPhysicsComponent*> mPhysicsComponentQueue;

	TWeakPtr<APawn> mPlayer;

	std::mutex mEventQueueMutex;


	/// <summary>
	/// Profiling Data
	/// </summary>
	double mTime_Tick_PrePhysics;
	double mTime_Update_Physics;
	double mTime_Physics_Event;
	double mTime_Tick_PostPhysics;
	double mTime_Update_Render_Items;

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
inline TWeakPtr<T> WWorld::SpawnActor()
{
	TSharedPtr<T> Actor = MakeShared<T>();
	int ActorId = (int)mAllActors.size();
	mAllActors.emplace_back(Actor);
	

	Actor->mActorId = ActorId;
	Actor->BeginPlay();

	return TWeakPtr<T>(Actor);
}
