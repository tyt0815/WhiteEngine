#include "Object.h"
#include "Component/SceneComponent.h"

WObject::WObject()
{
}


FWObjectManager::FWObjectManager()
{

}

FWObjectManager::~FWObjectManager()
{

}

void FWObjectManager::Tick(float Delta)
{
	for (size_t i = 0; i < mPool.GetPoolSize(); ++i)
	{
		if (mPool.IsUsed(i))
		{
			mPool.GetItemRef(i)->Tick(Delta);
		}
	}
	for (size_t i = 0; i < mAllRootComponent.GetPoolSize(); ++i)
	{
		if (mAllRootComponent.IsUsed(i))
		{
			mAllRootComponent.GetItemRef(i)->UpdateRecursive();
		}
	}
}