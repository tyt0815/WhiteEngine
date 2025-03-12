#pragma once
#include <memory>
#include "DirectX/DXMath.h"
#include "Utility/Class.h"
#include "Utility/Pool.h"

class AActor;
class WSceneComponent;
class WPrimitiveComponent;

class WObject
{
public:
	WObject();
	virtual ~WObject() = default;
	virtual void Tick(float Delta) {};
private:
	std::uint64_t mPoolId = 0;
public:
	inline std::uint64_t GetObjectId() const
	{
		return mPoolId;
	}
	friend class FWObjectManager;
};

class FWObjectManager
{
	SINGLETON(FWObjectManager);
public:
	void Tick(float Delta);
	template<typename T>
	T* CreateWObject();

private:
	TPool<std::shared_ptr<WObject>> mPool;
	TPool<WSceneComponent*> mAllRootComponent;
	//TPool<WPrimitiveComponent*> mAllPrimitiveComponent;
public:
	inline std::uint64_t GetNextPoolId() const
	{
		return mPool.GetNextIndex();
	}
	inline const TPool<std::shared_ptr<WObject>>& GetAllWObject() const
	{
		return mPool;
	}
	inline std::uint64_t RegisterRootComponent(WSceneComponent* Component)
	{
		return mAllRootComponent.Register(Component);
	}
	inline void RemoveRootComponent(std::uint64_t Id)
	{
		mAllRootComponent.Remove(Id);
	}
	//inline std::uint64_t GetNextPrimitivePoolId() const
	//{
	//	return mAllPrimitiveComponent.GetNextIndex();
	//}
	//inline std::uint64_t RegisterPrimitiveComponent(WPrimitiveComponent* Component)
	//{
	//	return mAllPrimitiveComponent.Register(Component);
	//}
	//inline void RemovePrimitiveComponent(std::uint64_t Id)
	//{
	//	mAllPrimitiveComponent.Remove(Id);
	//}
	//inline TPool<WPrimitiveComponent*>& GetAllPrimitiveComponentsRef()
	//{
	//	return mAllPrimitiveComponent;
	//}

	friend class WObject;
};

template<typename T>
inline T* FWObjectManager::CreateWObject()
{
	size_t id = mPool.Register(std::make_shared<T>());
	WObject* Object = mPool.GetItem(id).get();
	Object->mPoolId = id;
	return dynamic_cast<T*>(Object);
}

inline FWObjectManager* GetWObjectManager()
{
	return FWObjectManager::GetInstance();
}