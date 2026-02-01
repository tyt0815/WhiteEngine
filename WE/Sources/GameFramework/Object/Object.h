#pragma once
#include "Utility/Memory.h"
#include "Asset/BlueprintAsset.h"

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
	virtual void OnDestroy();

	virtual void OnActivate();

	virtual void OnDeactivate();

	void LoadWProperties(const TArray<BlueprintAsset::FProperty>& Properties);

	virtual void LoadWInitializers(const TArray<BlueprintAsset::FInitializer>& Initializers);

	template <typename T>
	void RegisterWProperty(const std::string& Name, T* ValuePtr);

private:
	ETickGroup mTickGroup = ETickGroup::ETG_None;

	ETickPriority mTickPriority = ETickPriority::ETP_None;

	std::unordered_map<std::string, void*> mBlueprintPropertiesMap;

	int mTickId = -1;

public:
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
	assert(mBlueprintPropertiesMap.count(Name) == 0);

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
