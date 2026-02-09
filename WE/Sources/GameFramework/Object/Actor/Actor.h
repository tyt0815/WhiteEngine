#pragma once

#include "Object.h"
#include "Asset/BlueprintAsset.h"
#include "GameFramework/Object/Component/SceneComponent.h"
#include "Component/PhysicsComponent.h"
#include "Utility/Class.h"
#include "Utility/Container.h"
#include "Physics/PhysicsCore.h"
#include "ActorFactory.h"
#include <d3d12.h>
#include <memory>
#include <variant>

extern const int gFrameResourcesNum;

class FMeshGeometry;
class FMaterial;
class WCameraComponent;

void ShowMessageBox(const std::string& Content);

void ReportParseError(const std::string& Type, const std::string& WrongValue);

void ApplySceneComponentDefaultAttributes(WSceneComponent* Comp, const std::unordered_map<std::string, std::string>& Attributes);

WWorld* GetWorld();

template<typename T> struct WPropertyTrait;
template<> struct WPropertyTrait<bool>
{
	static bool Parse(const std::string& String)
	{
		std::string LowerStr = String;
		std::transform(LowerStr.begin(), LowerStr.end(), LowerStr.begin(), ::tolower);

		if (LowerStr == "true" || LowerStr == "1") return true;
		if (LowerStr == "false" || LowerStr == "0") return false;
		ReportParseError("Bool (true/false/1/0)", String);
		return false;
	}
};
template<> struct WPropertyTrait<int>
{
	static int Parse(const std::string& String)
	{
		try {
			return std::stoi(String);
		}
		catch (...) {
			ReportParseError("Int", String);
			return 0;
		}
	}
};
template<> struct WPropertyTrait<float>
{
	static float Parse(const std::string& String)
	{
		try {
			return std::stof(String);
		}
		catch (...) {
			ReportParseError("Float", String);
			return 0.0f;
		}
	}
};
template<> struct WPropertyTrait<XMFLOAT3>
{
	static XMFLOAT3 Parse(const std::string& String)
	{
		XMFLOAT3 Float3 = { 0.f, 0.f, 0.f };

		int Result = sscanf_s(String.c_str(), "(%f, %f, %f)", &Float3.x, &Float3.y, &Float3.z);

		if (Result < 3)
		{
			ReportParseError("Float3 (x, y, z)", String);
		}
		return Float3;
	}
};
template<> struct WPropertyTrait<std::string>
{
	static std::string Parse(const std::string& String)
	{
		return String;
	}
};

class AActor : public WObject
{
	typedef WObject Super;
public:
	const unsigned int mActorCounter;

	AActor();

	virtual ~AActor() {};

	virtual void Tick(float DeltaSecond) override;

	virtual void Destroy() override;

	virtual void Activate() override;

	virtual void Deactivate() override;

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

	using WProperty = std::variant<bool*, int*, float*, XMFLOAT3*, std::string*>;
	using WVariantValue = std::variant<bool, int, float, XMFLOAT3, std::string>;
	using WPropertiesMap = std::unordered_map<std::string, WProperty>;

	class WEvent
	{
	public:
		std::string Name;

		std::function<void(const WEvent* Event, const WAttributesMap&)> Initializer;

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

protected:
	virtual void LoadWConfigs(const std::unordered_map<std::string, WAttributesMap>& Configs) {}

	void LoadWEvent(AActor::WEvent* Event, FBlueprintEventNode* EventNode);

	virtual void OnLoadWComponent(struct FBlueprintComponentNode* CompNode, WSceneComponent* Comp) {}

	void RegisterWComponentFactory(const std::string& Type, WComponentFactory Lambda);

	void RegisterWActionFactory(const std::string Name, WActionFactoryFunc Lambda);

	const WEvent* RegisterWEvent(const std::string& Name);

	void RegisterWEvent(const std::string& Name, std::function<void(const WEvent* Event, const WAttributesMap&)> InitializerFunc);

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

	void SetWProperty(const std::string& Name, WVariantValue Value);

private:
	void LoadWComponent_Internal(struct FBlueprintComponentNode* Comp, WSceneComponent* Parent);

	void LoadWEvents(const TArray<TSharedPtr<FBlueprintEventNode>>& Events);

	void RegisterWComponent(const std::string& Name, WSceneComponent* Comp);

	WEvent* RegisterWEvent_Internal(const std::string& Name);

	std::unordered_map<std::string, WComponentFactory> mWComponentFactoryMap;

	std::unordered_map<std::string, WSceneComponent*> mWComponentsMap;

	std::unordered_map<std::string, WActionFactoryFunc> mWActionFactoryMap;

	WEventsMap mWEventsMap;

	WPropertiesMap mWPropertiesMap;

	struct FOnTimeEvent
	{
		const WEvent* Event;
		float Time;
	};

	TArray<TSharedPtr<FOnTimeEvent>> mOnTimeEvents;
	int mOnTimeEventIndex = 0;

	const WEvent* mOnSpawnEvent;

	const WEvent* mOnDestroyEvent;

public:
	__forceinline WSceneComponent* GetWComponent(const std::string& Name) const
	{
		return mWComponentsMap.at(Name);
	}

	__forceinline WProperty GetWProperty(const std::string& Name) const
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

template <typename T>
bool ExtractAttribute(const std::unordered_map<std::string, std::string>& Attrs, const std::string& Key, T& Target)
{
	auto it = Attrs.find(Key);
	if (it != Attrs.end())
	{
		Target = WPropertyTrait<T>::Parse(it->second);
		return true;
	}
	return false;
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