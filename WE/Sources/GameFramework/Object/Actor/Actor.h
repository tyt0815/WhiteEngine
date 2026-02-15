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
#include <variant>
#include <sstream>
#include <queue>

extern const int gFrameResourcesNum;

class FMeshGeometry;
class FMaterial;
class WCameraComponent;
class FBlueprintAsset;

void ShowMessageBox(const std::string& Content);

void ShowMessageBox(const std::wstring& Content);

WWorld* GetWorld();

class AActor : public WObject
{
	typedef WObject Super;
public:
	const unsigned int mActorCounter;

	AActor();

	virtual ~AActor() {};

	virtual void Tick(float DeltaSecond) override;

	virtual void Destroy() override;

protected:
	virtual void OnDestroy() override;

	virtual void OnActivate() override;

	virtual void OnDeactivate() override;

public:
	virtual void BeginPlay();

	template<typename T>
	T* CreateComponent();

	template<typename T>
	T* CreateComponentByFactory(const std::string Name);

	template<typename T>
	T* GetComponent();

	XMFLOAT3 GetForwardVector() const;

	XMFLOAT3 GetRightVector() const;

	XMFLOAT3 GetUpVector() const;

	XMFLOAT4 GetActorQuaternion();

	void SetRootComponent(TWeakPtr<WSceneComponent> Component);

	void SetActorTransform(FTransform Transform);

protected:
	virtual void OnCreateComponent(WActorComponent* Comp);

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

	float mElapsedTime = 0;

	float mDeltaTime = 0;

	/////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// Blueprint Section
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////
protected:
	void AddWComponent(const std::string& Name, WActorComponent* Comp);

	virtual void LoadBlueprint(const FBlueprintAsset* Asset);

private:
	std::unordered_map<std::string, WActorComponent*> mWComponentsMap;

public:
	template<typename T>
	__forceinline T* GetWComponent(const std::string& Name) const
	{
		return dynamic_cast<T*>(mWComponentsMap.at(Name));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////
	// Spline Section
	/////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////
private:
	struct FSplineFollowInfo
	{
		WSceneComponent* Target = nullptr;
		class WSplineComponent* Spline = nullptr;
		float Duration = 1;
		float ElapsedTime = 0;
		bool bLoop = false;
		bool bUseRotation = true;
	};

	TArray<FSplineFollowInfo> mSplineFollowInfos;

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
		return mRootComponent.lock()->GetRelativeLocation();
	}

	__forceinline void SetActorLocation(XMFLOAT3 Location)
	{
		mRootComponent.lock()->SetRelativeLocation(Location);
	}

	__forceinline XMFLOAT3 GetActorRotation() const
	{
		return mRootComponent.lock()->GetRelativeRotation();
	}

	__forceinline void SetActorRotation(XMFLOAT3 Rotation)
	{
		mRootComponent.lock()->SetRelativeRotation(Rotation);
	}

	__forceinline XMFLOAT3 GetActorScale() const
	{
		return mRootComponent.lock()->GetRelativeScale();
	}

	__forceinline XMMATRIX GetWorldMatrix() const
	{
		return mRootComponent.lock()->GetWorldMatrix();
	}

	__forceinline void SetActorScale(XMFLOAT3 Scale)
	{
		mRootComponent.lock()->SetRelativeScale(Scale);
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
	friend class FBlueprintAsset;
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