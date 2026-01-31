#pragma once

#include "Object.h"
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

class AActor : public WObject
{
	typedef WObject Super;
public:
	AActor();

	virtual ~AActor() {};

	virtual void BeginPlay();

	template<typename T>
	TWeakPtr<T> CreateComponent();

	void SetRootComponent(TWeakPtr<WSceneComponent> Component);

	void SetActorTransform(FTransform Transform);

	XMFLOAT3 GetForwardVector() const;

	XMFLOAT3 GetRightVector() const;

	XMFLOAT3 GetUpVector() const;

	XMFLOAT4 GetActorQuaternion();

	void Destroy() override;

	void Activate() override;

	void Deactivate() override;

protected:
	virtual void OnDestroy() override;

	virtual void OnActivate() override;

	virtual void OnDeactivate() override;

	void LoadBlueprint(const std::wstring& Name);

	std::unordered_map<std::string, void*> mBlueprintMap;

private:
	void UpdateRecursive();

	void BeginComponents();

	TArray<TSharedPtr<WActorComponent>> mAllComponents;

	TArray<TWeakPtr<WSceneComponent>> mAllSceneComponent;

	TArray<TWeakPtr<WActorComponent>> mAllNoneSceneComponent;

	TArray<TWeakPtr<WPhysicsComponent>> mAllPhysicsComponents;

	TWeakPtr<WSceneComponent> mRootComponent;

	int mActorId = -1;

	int mActiveActorQueueId = -1;

	bool mbPendingKill = false;

public:

	__forceinline TWeakPtr<WSceneComponent> GetRootComponent() const
	{
		return mRootComponent;
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
};

template<typename T>
__forceinline TWeakPtr<T> AActor::CreateComponent()
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
