#pragma once

#include "GameFramework/Object/Component/PrimitiveComponent.h"
#include "Utility/Class.h"
#include "Utility/Container.h"
#include "Physics/PhysicsCore.h"

#include <d3d12.h>
#include <memory>

extern const int gFrameResourcesNum;

class FMeshGeometry;
class FMaterial;
class WCameraComponent;


class AActor
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
	T* CreateComponent();

	void SetRootComponent(WSceneComponent* Component);

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

	void SetupComponent(WActorComponent* Component);

	void SetupSceneComponent(WSceneComponent* Component);

	TUnorderedArray<std::unique_ptr<WActorComponent>> mAllComponents;

	TUnorderedArray<WSceneComponent*> mAllSceneComponent;

	TUnorderedArray<WActorComponent*> mAllNoneSceneComponent;

	TUnorderedArray<WPrimitiveComponent*> mAllPrimitiveComponents;

	WSceneComponent* mRootComponent = nullptr;

	WWorld* mWorld = nullptr;

	size_t mActorId = -1;

	bool mbPendingKill = false;

public:
	inline TUnorderedArray<std::unique_ptr<WActorComponent>>& GetAllComponents()
	{
		return mAllComponents;
	}

	inline WSceneComponent* GetRootComponent() const
	{
		return mRootComponent;
	}

	inline FTransform GetActorTransform() const
	{
		return mRootComponent->GetLocalTransform();
	}

	inline XMFLOAT3 GetActorLocation() const
	{
		return mRootComponent->GetLocalLocation();
	}

	inline void SetActorLocation(XMFLOAT3 Location)
	{
		mRootComponent->SetLocalLocation(Location);
	}

	inline XMFLOAT3 GetActorRotation() const
	{
		return mRootComponent->GetLocalRotation();
	}

	inline void SetActorRotation(XMFLOAT3 Rotation)
	{
		mRootComponent->SetLocalRotation(Rotation);
	}

	inline XMFLOAT3 GetActorScale() const
	{
		return mRootComponent->GetLocalScale();
	}

	inline void SetActorScale(XMFLOAT3 Scale)
	{
		mRootComponent->SetLocalScale(Scale);
	}

	inline WWorld* GetWorld() const
	{
		return mWorld;
	}

	inline void SetWorld(WWorld* World)
	{
		mWorld = World;
	}

	inline void MarkPendingKill()
	{
		mbPendingKill = true;
	}

	inline bool IsValid() const
	{
		return !mbPendingKill;
	}

	inline size_t GetActorId() const
	{
		return mActorId;
	}

	inline void SetActorId(size_t Id)
	{
		mActorId = Id;
	}
};

template<typename T>
inline T* AActor::CreateComponent()
{
	size_t Index = mAllComponents.Add(std::make_unique<T>());
	WActorComponent* ActorComp = mAllComponents[Index].get();
	ActorComp->SetOwner(this);
	
	WSceneComponent* SceneComp = dynamic_cast<WSceneComponent*>(ActorComp);
	if (SceneComp != nullptr)
	{
		mAllSceneComponent.Add(SceneComp);

		if (WPrimitiveComponent* PrimitiveComp = dynamic_cast<WPrimitiveComponent*>(SceneComp))
		{
			mAllPrimitiveComponents.Add(PrimitiveComp);
		}		
	}
	else
	{
		mAllNoneSceneComponent.Add(ActorComp);
	}

	T* Comp = dynamic_cast<T*>(ActorComp);

	assert(mWorld == nullptr);
	
	return Comp;
}
