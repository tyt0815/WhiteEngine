#pragma once

#include "Object.h"
#include "GameFramework/Object/Component/SceneComponent.h"
#include "Component/PhysicsComponent.h"
#include "Utility/Class.h"
#include "Utility/Container.h"
#include "Physics/PhysicsCore.h"
#include "ActorFactory.h"
#include <d3d12.h>
#include <memory>

extern const int gFrameResourcesNum;

class FMeshGeometry;
class FMaterial;
class WCameraComponent;
class FBlueprintAsset;

class AActor : public WObject
{
	typedef WObject Super;
public:
	const unsigned int mActorCounter;

	AActor();

	virtual ~AActor() {};

	virtual void Destroy() override;

	virtual void Activate() override;

	virtual void Deactivate() override;

protected:
	virtual void OnDestroy() override;

	virtual void OnActivate() override;

	virtual void OnDeactivate() override;

	virtual void OnCreateComponent(WActorComponent* Comp);

public:
	virtual void BeginPlay();

	void LoadBlueprint(const FBlueprintAsset* Blueprint);

	template<typename T>
	T* CreateComponent();

	template<typename T>
	T* CreateComponentByFactory(const std::string Name);

	template<typename T>
	T* GetComponent();

	void SetRootComponent(TWeakPtr<WSceneComponent> Component);

	void SetActorTransform(FTransform Transform);

	XMFLOAT3 GetForwardVector() const;

	XMFLOAT3 GetRightVector() const;

	XMFLOAT3 GetUpVector() const;

	XMFLOAT4 GetActorQuaternion();

protected:
	using FBlueprintAttributesMap = std::unordered_map<std::string, std::string>;

	virtual void LoadBlueprintAttribute(const FBlueprintAttributesMap& Attributes) {}

	void RegisterToComponentFactory(const std::string& Type, std::function<WSceneComponent* (const FBlueprintAttributesMap&)> Lambda);

private:
	void UpdateRecursive();

	void BeginComponents();

	void LoadBlueprintComponent_Internal(struct FComponentNode* Comp, WSceneComponent* Parent);

	void RegisterComponentByName(const std::string& Name, WSceneComponent* Comp);

	TArray<TSharedPtr<WActorComponent>> mAllComponents;

	TArray<TWeakPtr<WSceneComponent>> mAllSceneComponent;

	TArray<TWeakPtr<WActorComponent>> mAllNoneSceneComponent;

	TArray<TWeakPtr<WPhysicsComponent>> mAllPhysicsComponents;

	TWeakPtr<WSceneComponent> mRootComponent;

	std::unordered_map<std::string, std::function<WSceneComponent* (const FBlueprintAttributesMap&)>> mComponentFactory;

	std::unordered_map<std::string, WSceneComponent*> mBlueprintComponents;

	int mActorId = -1;

	int mActiveActorQueueId = -1;

	bool mbPendingKill = false;

public:

	__forceinline WSceneComponent* GetRootComponent() const
	{
		return mRootComponent.lock().get();
	}

	__forceinline FTransform GetActorTransform() const
	{
		return mRootComponent.lock()->GetLocalTransform();
	}

	__forceinline XMFLOAT3 GetActorLocation() const
	{
		return mRootComponent.lock()->GetLocalLocation();
	}

	__forceinline void SetActorLocation(XMFLOAT3 Location)
	{
		mRootComponent.lock()->SetLocalLocation(Location);
	}

	__forceinline XMFLOAT3 GetActorRotation() const
	{
		return mRootComponent.lock()->GetLocalRotation();
	}

	__forceinline void SetActorRotation(XMFLOAT3 Rotation)
	{
		mRootComponent.lock()->SetLocalRotation(Rotation);
	}

	__forceinline XMFLOAT3 GetActorScale() const
	{
		return mRootComponent.lock()->GetLocalScale();
	}

	__forceinline void SetActorScale(XMFLOAT3 Scale)
	{
		mRootComponent.lock()->SetLocalScale(Scale);
	}

	__forceinline bool IsPendingKill() const
	{
		return mbPendingKill;
	}

	__forceinline bool IsActivated() const
	{
		return mActiveActorQueueId >= 0;
	}

	friend class WWorld;
	friend class FActorFactory;
};

template<typename T>
__forceinline T* AActor::CreateComponent()
{
	static_assert(IsDerivedFrom<WActorComponent, T>());

	TSharedPtr<T> CompT = MakeShared<T>();
	mAllComponents.emplace_back(CompT);

	OnCreateComponent(CompT.get());	
	
	return CompT.get();
}

template<typename T>
inline T* AActor::CreateComponentByFactory(const std::string Name)
{
	static_assert(IsDerivedFrom<WActorComponent, T>());

	TSharedPtr<T> CompT = FComponentFactory::CreateComponent<T>(Name);
	mAllComponents.emplace_back(CompT);

	OnCreateComponent(CompT.get());

	return CompT.get();
}

template<typename T>
inline T* AActor::GetComponent()
{
	for (const TSharedPtr<WActorComponent>& Comp : mAllComponents)
	{
		if (T* TComp = dynamic_cast<T*>(Comp.get()))
		{
			return TComp;
		}
	}
	return nullptr;
}

REGISTER_ACTOR(AActor);