#pragma once

#include "GameFramework/Object/Pawn/Pawn.h"
#include "GameFramework/RenderItemProxy.h"
#include "Utility/Container.h"
#include "Utility/Class.h"

class WWorld
{
public:
	WWorld();

	virtual ~WWorld();

public:
	virtual void Tick(float Delta);

	void SetPlayer(APawn* Player);

	template<typename T>
	T* SpawnActor();

	void DestroyActor(AActor* Actor);

	void DrawDebugLine(XMFLOAT3 Location, XMFLOAT4 Color, float LifeSpan);

private:
	void FlushDestroyQueue();

	TUnorderedArray<std::unique_ptr<AActor>> mAllActors;

	std::vector<AActor*> DestroyQueue;

	APawn* mPlayer = nullptr;

	FRenderItemProxy mRenderItemProxy;

public:
	inline APawn* GetPlayer() const
	{
		return mPlayer;
	}

	inline WCameraComponent* GetPlayerCamera() const
	{
		return mPlayer->GetCameraComponent();
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

	inline FRenderItemProxy* GetRenderItemProxyPtr()
	{
		return &mRenderItemProxy;
	}
};

extern WWorld* gWorld;

inline WWorld* GetWorld()
{
	return gWorld;
}

template<typename T>
inline T* WWorld::SpawnActor()
{
	UINT Index = (UINT)mAllActors.Add(std::make_unique<T>());
	T* Actor = dynamic_cast<T*>(mAllActors[Index].get());
	Actor->SetWorld(this);
	Actor->SetActorId(Index);

	if (Actor->GetRootComponent() == nullptr)
	{
		WSceneComponent* DummyRoot = Actor->CreateComponent<WSceneComponent>();
		Actor->SetRootComponent(DummyRoot);
	}

	Actor->BeginPlay();

	return Actor;
}
