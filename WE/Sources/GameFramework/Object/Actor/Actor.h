#pragma once

#include "Object.h"
#include "Asset/BlueprintAsset.h"
#include "GameFramework/Object/Component/SceneComponent.h"
#include "Component/PhysicsComponent.h"
#include "Utility/Class.h"
#include "Utility/Container.h"
#include "Physics/PhysicsCore.h"
#include "ActorFactory.h"
#include "WEngineTypes.h"
#include <d3d12.h>
#include <memory>
#include <variant>
#include <sstream>

extern const int gFrameResourcesNum;

class FMeshGeometry;
class FMaterial;
class WCameraComponent;

void ShowMessageBox(const std::string& Content);

void ShowMessageBox(const std::wstring& Content);

void ReportParseError(const std::string& Type, const std::string& WrongValue);

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

	float mElapsedTime = 0;

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
public:
	void LoadBlueprint(const FBlueprintAsset* Blueprint);

	using WComponentFactory = std::function<WSceneComponent* (const WAttributesMap&)>;

	using WAction = std::function<void()>;
	using WActionFactoryFunc = std::function<WAction(const WAttributesMap&)>;

	using WPropertiesMap = std::unordered_map<std::string, WSourceRef>;

	using WFunction = std::function<WEvalValue()>;

	class WEvent 
	{
	public:
		void Dispatch() const 
		{
			for (const auto& Action : mActions) Action();
		}
		void AddAction(const WAction& Action) 
		{ 
			mActions.push_back(Action);
		}

	private:
		std::vector<WAction> mActions;
	};
	using WEventLoader = std::function<WEvent*(const WAttributesMap&)>;

	WEvalValue ExecuteWFunction(const std::string& Name);

	template<typename T>
	T ExecuteWFunction(const std::string& FullName)
	{
		auto it = mWFunctionsMap.find(FullName);

		if (it != mWFunctionsMap.end())
		{
			try 
			{
				// 실행 후 T 타입으로 형변환하여 반환
				return std::get<T>(it->second());
			}
			catch (const std::bad_variant_access&) {
				Log("Return Type Mismatch for: " + FullName);
			}
		}
		else
		{
			Log("Function Key Not Found in Map: " + FullName);
		}

		return T{};
	}

protected:
	virtual void LoadWConfigs(const std::unordered_map<std::string, WAttributesMap>& Configs);

	void LoadWEvent(AActor::WEvent* Event, const TArray<TSharedPtr<FBlueprintActionNode>>& Actions);

	virtual void ApplyWComponentCommonAttribute(const WAttributesMap& Attributes, WSceneComponent* Comp);

	void RegisterWComponentCommonFunction(const std::string Name, WSceneComponent* Comp);

	void RegisterWComponentFactory(const std::string& Type, WComponentFactory Lambda);

	void RegisterWActionFactory(const std::string Name, WActionFactoryFunc Lambda);

	void RegisterSystemEvent(const std::string& Name, WEvent* Event);

	void RegisterSystemEvent(const std::string& Name, WEventLoader Loader);

	void RegisterWFunction(const std::string& Name, WFunction Lambda);

	template<typename T>
	void RegisterWProperty(const std::string& Name, T* Property)
	{
		if (mWPropertiesMap.count(Name) > 0)
		{
			ShowMessageBox("Already registered property:\n" + Name);
			assert(false);
		}

		mWPropertiesMap[Name] = Property;
	}

	WEvent* GenerateWEvent(std::unordered_map<std::string, TSharedPtr<WEvent>>& Container, const std::string& Name);

	WEvent* GenerateWEvent(std::vector<TSharedPtr<WEvent>>& Container);

	void SetWProperty(const std::string& Name, WEvalValue Value);

private:
	void LoadWComponent_Internal(struct FBlueprintComponentNode* Comp, WSceneComponent* Parent);

	void LoadWEvents(const TArray<TSharedPtr<FBlueprintEventNode>>& SystemEvents, const TArray<TSharedPtr<FBlueprintEventNode>>& CustomEvents);

	void RegisterWComponent(const std::string& Name, WSceneComponent* Comp);

	std::unordered_map<std::string, WComponentFactory> mWComponentFactoryMap;

	std::unordered_map<std::string, WSceneComponent*> mWComponentsMap;

	std::unordered_map<std::string, WActionFactoryFunc> mWActionFactoryMap;

	std::unordered_map<std::string, WEventLoader> mSystemEventLoaders;

	std::unordered_map<std::string, TSharedPtr<WEvent>> mCustomEventsMap;
	std::unordered_map<std::string, TSharedPtr<WEvent>> mOnActivateEventsMap;
	std::unordered_map<std::string, TSharedPtr<WEvent>> mOnDeactivateEventsMap;

	std::unordered_map<std::string, WFunction> mWFunctionsMap;

	TArray<TUniquePtr<WEvalValue>> mCustomWProperies;

	WPropertiesMap mWPropertiesMap;

	struct FOnTimeEvent
	{
		WEvent Event;
		float Time;
	};

	TArray<TSharedPtr<FOnTimeEvent>> mOnTimeEvents;
	int mOnTimeEventIndex = 0;

	WEvent mOnSpawnEvent;

	WEvent mOnDestroyEvent;

	float mLifeSpan = 0;

public:
	template<typename T>
	__forceinline T* GetWComponent(const std::string& Name) const
	{
		return dynamic_cast<T*>(mWComponentsMap.at(Name));
	}

	__forceinline WSceneComponent* GetWComponent(const std::string& Name) const
	{
		return GetWComponent<WSceneComponent>(Name);
	}

	__forceinline WSourceRef GetWPropertyPtr(const std::string& Name) const
	{
		return mWPropertiesMap.at(Name);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////
	// Blueprint Section End
	/////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////

	int mActorId = -1;

	int mActiveActorQueueId = -1;

	bool mbPendingKill = false;

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

	__forceinline XMMATRIX GetWorldMatrix() const
	{
		return mRootComponent.lock()->GetWorldMatrix();
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