#pragma once
#include <memory>
#include "DirectX/DXMath.h"
#include "Utility/Class.h"
#include "Utility/Pool.h"

class WObject
{
public:
	WObject() = default;
	virtual ~WObject() = default;
	virtual void Tick(float Delta) {};
};

class FWObjectManager
{
	SINGLETON(FWObjectManager);
public:
	template<typename T>
	T* CreateWObject();

private:
	TPool<std::shared_ptr<WObject>> mPool;
public:
	const TPool<std::shared_ptr<WObject>>& GetAllWObject() const
	{
		return mPool;
	}
};

template<typename T>
inline T* FWObjectManager::CreateWObject()
{
	std::shared_ptr<T> Object = std::make_shared<T>();
	mPool.Register(Object);
	return Object.get();
}

inline FWObjectManager* GetWObjectManager()
{
	return FWObjectManager::GetInstance();
}