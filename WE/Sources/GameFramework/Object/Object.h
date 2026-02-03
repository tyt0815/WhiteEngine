#pragma once
#include "Utility/Memory.h"
#include "Asset/BlueprintAsset.h"
#include <functional>

extern class WWorld* g_World;

enum class ETickGroup : unsigned int
{
	ETG_PrePhysics = 0,
	ETG_PostPhysics,
	ETG_None
};

enum class ETickPriority : unsigned int
{
	ETP_High = 0,
	ETP_Middle,
	ETP_Low,
	ETP_None
};

class WObject : public std::enable_shared_from_this<WObject>
{
public:
	WObject();

	virtual void Tick(float DeltaSecond);

	void SetTickGroup(ETickGroup TickGroup, ETickPriority TickPriority);

	virtual void Destroy() = 0;

	virtual void Activate() = 0;

	virtual void Deactivate() = 0;

	template <typename T>
	T* GetWPropertyPtr(const std::string& Name);

	template <typename T>
	T* GetWPropertyPtrSafe(const std::string& Name);

	template <typename T>
	void SetWProperty(const std::string& Name, const T& Value);

	template <typename T>
	void RegisterWPropertySafe(const std::string& Name, T* ValuePtr);

protected:
	struct WFunctionParameter
	{
		std::string Name;
		TSharedPtr<void> Value;

		template<typename T>
		T Get() const
		{
			if (!Value)
			{
				assert(false);
				return T();
			}

			TSharedPtr<T> TypedPtr = std::static_pointer_cast<T>(Value);

			if (!TypedPtr)
			{
				assert(false && "Invalid Type Cast in WFunctionParameter::Get");
				return T();
			}

			return *TypedPtr;
		}
	};

	using WFunctionParams = TArray<TSharedPtr<WFunctionParameter>>;
	using WFunctionReturn = TSharedPtr<void>;
	using WFunction = std::function<WFunctionReturn(WFunctionParams)>;

	class WEvent
	{
	public:
		void LoadEvent(WObject* Context, const BlueprintAsset::FEventNode& Event);

		WFunctionReturn CallFunction(WObject* Context, const BlueprintAsset::FFunctionNode* FuncNode);

		void Dispatch() const;

	private:
		TArray<std::function<void()>> mFunctions;
	};

	virtual void OnDestroy();

	virtual void OnActivate();

	virtual void OnDeactivate();

	void LoadWProperties(const TArray<BlueprintAsset::FProperty>& Properties);

	void LoadEvents(WObject* Context, const TArray<BlueprintAsset::FEventNode>& Events);

	template <typename T>
	void RegisterWProperty(const std::string& Name, T* ValuePtr);

	void RegisterWFunction(const std::string& Name, WFunction Func);

private:
	ETickGroup mTickGroup = ETickGroup::ETG_None;

	ETickPriority mTickPriority = ETickPriority::ETP_None;

	std::unordered_map<std::string, void*> mBlueprintPropertiesMap;

	std::unordered_map<std::string, WFunction> mWFunctions;

	std::unordered_map<std::string, WEvent> mWEvents;

	const WEvent* mTickEvent;

	int mTickId = -1;

public:
	__forceinline const WEvent* RegisterEvent(const std::string& Name)
	{
		return &mWEvents[Name];
	}

	template<typename T>
	__forceinline TWeakPtr<T> GetWeakPtr()
	{
		return Cast<T>(shared_from_this());
	}

	__forceinline WWorld* GetWorld() const
	{
		return g_World;
	}


	friend class WWorld;
};

template<typename T>
inline void WObject::RegisterWProperty(const std::string& Name, T* ValuePtr)
{
	if (mBlueprintPropertiesMap.count(Name) > 0)
	{
		assert(mBlueprintPropertiesMap[Name] != ValuePtr && "Alreay registered Name");
	}

	mBlueprintPropertiesMap[Name] = ValuePtr;
}

template<typename T>
inline T* WObject::GetWPropertyPtr(const std::string& Name)
{
	return static_cast<T*>(mBlueprintPropertiesMap.at(Name));
}

template<typename T>
inline T* WObject::GetWPropertyPtrSafe(const std::string& Name)
{
	return mBlueprintPropertiesMap.count(Name) > 0 ? GetWPropertyPtr<T>(Name) : nullptr;
}

template<typename T>
inline void WObject::SetWProperty(const std::string& Name, const T& Value)
{
	T* Ref = GetWPropertyPtr<T>(Name);
	*Ref = Value;
}

template<typename T>
inline void WObject::RegisterWPropertySafe(const std::string& Name, T* ValuePtr)
{
	if (mBlueprintPropertiesMap.count(Name) == 0)
	{
		RegisterWProperty<T>(Name, ValuePtr);
	}
	else
	{
		mBlueprintPropertiesMap[Name] = ValuePtr;
	}
}
