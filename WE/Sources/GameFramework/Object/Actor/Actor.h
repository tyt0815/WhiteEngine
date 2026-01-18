#pragma once

#include "GameFramework/Object/Component/SceneComponent.h"
#include "Component/PhysicsComponent.h"
#include "Utility/Class.h"
#include "Utility/Container.h"
#include "Physics/PhysicsCore.h"

#include <d3d12.h>
#include <memory>

extern const int gFrameResourcesNum;

class FMeshGeometry;
class FMaterial;
class WCameraComponent;


class AActor : public std::enable_shared_from_this<AActor>
{
public:
	AActor();

	virtual ~AActor() {};

	virtual void BeginPlay();

	virtual void Tick_PrePhysics(float Delta);

	virtual void Tick_PostPhysics(float Delta);

	void UpdateComponentsToPhysics();

	void UpdateComponentsFromPhysics();

	template<typename T>
	TWeakPtr<T> CreateComponent();

	void SetRootComponent(TWeakPtr<WSceneComponent> Component);

	void SetActorTransform(FTransform Transform);

	XMFLOAT3 GetFowardVector() const;

	XMFLOAT3 GetRightVector() const;

	XMFLOAT3 GetUpVector() const;

	void Destroy();

protected:

private:
	void BeginComponents();

	void TickComponents_PrePhysics(float Delta);

	void TickComponents_PostPhysics(float Delta);

	TArray<TSharedPtr<WActorComponent>> mAllComponents;

	TArray<TWeakPtr<WSceneComponent>> mAllSceneComponent;

	TArray<TWeakPtr<WActorComponent>> mAllNoneSceneComponent;

	TArray<TWeakPtr<WPhysicsComponent>> mAllPhysicsComponents;

	TWeakPtr<WSceneComponent> mRootComponent;

	WWorld* mWorld;

	UINT64 mActorId = -1;

	bool mbPendingKill = false;

public:
	__forceinline TWeakPtr<AActor> GetWeakPtr()
	{
		return GetWeakPtr<AActor>();
	}

	template<typename T>
	__forceinline TWeakPtr<T> GetWeakPtr()
	{
		return Cast<T>(shared_from_this());
	}

	inline TWeakPtr<WSceneComponent> GetRootComponent() const
	{
		return mRootComponent;
	}

	inline FTransform GetActorTransform() const
	{
		return mRootComponent.lock()->GetLocalTransform();
	}

	inline XMFLOAT3 GetActorLocation() const
	{
		return mRootComponent.lock()->GetLocalLocation();
	}

	inline void SetActorLocation(XMFLOAT3 Location)
	{
		mRootComponent.lock()->SetLocalLocation(Location);
	}

	inline XMFLOAT3 GetActorRotation() const
	{
		return mRootComponent.lock()->GetLocalRotation();
	}

	inline void SetActorRotation(XMFLOAT3 Rotation)
	{
		mRootComponent.lock()->SetLocalRotation(Rotation);
	}

	inline XMFLOAT3 GetActorScale() const
	{
		return mRootComponent.lock()->GetLocalScale();
	}

	inline void SetActorScale(XMFLOAT3 Scale)
	{
		mRootComponent.lock()->SetLocalScale(Scale);
	}

	inline WWorld* GetWorld() const
	{
		return mWorld;
	}

	inline void SetWorld(WWorld* World)
	{
		mWorld = World;
	}

	inline bool IsPendingKill() const
	{
		return mbPendingKill;
	}

	inline void MarkPendingKill()
	{
		mbPendingKill = true;
	}

	inline UINT64 GetActorId() const
	{
		return mActorId;
	}

	inline void SetActorId(UINT64 Id)
	{
		mActorId = Id;
	}
};

template<typename T>
inline TWeakPtr<T> AActor::CreateComponent()
{
	static_assert(IsDerivedFrom<WActorComponent, T>());

	TSharedPtr<T> CompT = MakeShared<T>();
	mAllComponents.emplace_back(CompT);

	if (TSharedPtr<WSceneComponent> SceneComp = Cast<WSceneComponent>(CompT))
	{
		mAllSceneComponent.emplace_back(SceneComp);
		if (TSharedPtr<WPhysicsComponent> PhysicsComp = Cast<WPhysicsComponent>(SceneComp))
		{
			mAllPhysicsComponents.emplace_back(PhysicsComp);
		}
	}
	else
	{
		mAllNoneSceneComponent.emplace_back(CompT);
	}
	
	return TWeakPtr<T>(CompT);
}
