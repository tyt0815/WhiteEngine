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

private:
	void UpdateRecursive();

	void BeginComponents();

	TArray<TSharedPtr<WActorComponent>> mAllComponents;

	TArray<TWeakPtr<WSceneComponent>> mAllSceneComponent;

	TArray<TWeakPtr<WActorComponent>> mAllNoneSceneComponent;

	TArray<TWeakPtr<WPhysicsComponent>> mAllPhysicsComponents;

	TWeakPtr<WSceneComponent> mRootComponent;


	/////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////
	// Blueprint Section
	/////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////
protected:
	using WAttributesMap = std::unordered_map<std::string, std::string>;

	using WComponentFactory = std::function<WSceneComponent* (const WAttributesMap&)>;

	using WAction = std::function<void()>;
	using WActionFactoryFunc = std::function<WAction(const WAttributesMap&)>;

	class WEvent
	{
	public:
		void Dispatch() const
		{
			for (const WAction& Action : mActions) Action();
		}

		void AddAction(WAction Action)
		{
			mActions.push_back(std::move(Action));
		}

	private:
		TArray<WAction> mActions;
	};
	using WEventsMap = std::unordered_map<std::string, TSharedPtr<WEvent>>;

	virtual void LoadWAttributes(const WAttributesMap& Attributes) {}

	void RegisterWComponentFactory(const std::string& Type, WComponentFactory Lambda);

	void RegisterWActionFactory(const std::string Name, WActionFactoryFunc Lambda);

	const WEvent* RegisterWEvent(const std::string& Name);

private:
	void LoadWComponent_Internal(struct FBlueprintComponentNode* Comp, WSceneComponent* Parent);

	void RegisterWComponent(const std::string& Name, WSceneComponent* Comp);

	std::unordered_map<std::string, WComponentFactory> mWComponentFactoryMap;

	std::unordered_map<std::string, WSceneComponent*> mWComponentsMap;

	std::unordered_map<std::string, WActionFactoryFunc> mWActionFactoryMap;

	WEventsMap mWEventsMap;

	const WEvent* mOnSpawnEvent;

	/////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////
	// Blueprint Section End
	/////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////

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

void ShowMessageBox(const std::string& Content);

bool ParseBool(const std::string& String);
int ParseInt(const std::string& String);
float ParseFloat(const std::string& String);
XMFLOAT3 ParseFloat3(const std::string& String);

template <typename TSetterFunc>
void ApplyAttribute(const std::unordered_map<std::string, std::string>& Attrs, const std::string& Key, TSetterFunc Setter)
{
	auto it = Attrs.find(Key);
	if (it != Attrs.end())
	{
		Setter(it->second);
	}
}

template <typename TParserFunc, typename TSetterFunc>
void ApplyAttribute(const std::unordered_map<std::string, std::string>& Attrs, const std::string& Key, TParserFunc Parser, TSetterFunc Setter)
{
	auto it = Attrs.find(Key);
	if (it != Attrs.end())
	{
		Setter(Parser(it->second));
	}
}