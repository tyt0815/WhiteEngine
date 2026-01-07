#pragma once

#include "GameFramework/Object/Component/SceneComponent.h"
#include "Utility/Class.h"
#include "Utility/Container.h"

#include <d3d12.h>
#include <memory>

extern const int gFrameResourcesNum;

class FMeshGeometry;
class FMaterial;
class WCameraComponent;

class AActor
{
public:
	virtual void Tick(float Delta);

	template<typename T>
	T* CreateComponent();

	void SetRootComponent(WSceneComponent* Component);

	XMFLOAT3 GetFowardVector() const;

	XMFLOAT3 GetRightVector() const;

	XMFLOAT3 GetUpVector() const;

private:
	void SetupComponent(WActorComponent* Component);

	void SetupSceneComponent(WSceneComponent* Component);

	TUnorderedArray<std::unique_ptr<WActorComponent>> mAllComponents;

	TUnorderedArray<WSceneComponent*> mAllSceneComponent;

	TUnorderedArray<WActorComponent*> mAllNoneSceneComponent;

	WSceneComponent* mRootComponent = nullptr;


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
	inline void SetActorTransform(FTransform Transform)
	{
		mRootComponent->SetLocalTransform(Transform);
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

};

template<typename T>
inline T* AActor::CreateComponent()
{
	size_t Index = mAllComponents.Add(std::make_unique<T>());
	WActorComponent* ActorComp = mAllComponents[Index].get();
	
	WSceneComponent* SceneComp = dynamic_cast<WSceneComponent*>(ActorComp);
	if (SceneComp != nullptr)
	{
		mAllSceneComponent.Add(SceneComp);
	}
	else
	{
		mAllNoneSceneComponent.Add(ActorComp);
	}

	T* Comp = dynamic_cast<T*>(ActorComp);

	return Comp;
}
