#pragma once

#include "Pawn/Pawn.h"
#include "GameFramework/RenderItemProxy.h"
#include "Utility/Container.h"
#include "Utility/Class.h"

class WPhysicsComponent;

struct FContactInfo
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

	void EnqueueOnBeginOverlapEvent(const FContactInfo& Info);

	void EnqueueOnHitEvent(const FContactInfo& Info);

	void ActivateActor(AActor* Actor);

	void DeactivateActor(AActor* Actor);

private:
	void FlushDestroyQueue();

	TArray<TSharedPtr<AActor>> mAllActors;

	std::vector<FContactInfo> mOnBeginOverlapEventQueue;

	std::vector<FContactInfo> mOnHitEventQueue;

	std::vector<TSharedPtr<AActor>> DestroyQueue;

	std::vector<AActor*> mActiveActorQueue;

	TWeakPtr<APawn> mPlayer;

	std::mutex mEventQueueMutex;

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

extern WWorld* gWorld;

inline WWorld* GetWorld()
{
	return gWorld;
}

template<typename T>
inline TWeakPtr<T> WWorld::SpawnActor()
{
	TSharedPtr<T> Actor = MakeShared<T>();
	UINT64 ActorId = mAllActors.size();
	mAllActors.emplace_back(Actor);
	

	Actor->mWorld = this;
	Actor->mActorId = ActorId;
	Actor->BeginPlay();
	Actor->Activate();

	return TWeakPtr<T>(Actor);
}
