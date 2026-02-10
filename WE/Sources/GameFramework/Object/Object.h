#pragma once
#include "Utility/Memory.h"
#include "Utility/Delegate.h"
#include <functional>

DECLARE_MULTICAST_DELEGATE(FOnActivate);
DECLARE_MULTICAST_DELEGATE(FOnDeactivate);

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

public:
	virtual void Tick(float DeltaSecond);

	void SetTickGroup(ETickGroup TickGroup, ETickPriority TickPriority);

	virtual void Destroy() = 0;

	void Activate();

	void Deactivate();

	FOnActivate mOnActivate;

	FOnDeactivate mOnDeactivate;

protected:

	virtual void OnDestroy();

	virtual void OnActivate();

	virtual void OnDeactivate();

private:
	ETickGroup mTickGroup = ETickGroup::ETG_None;

	ETickPriority mTickPriority = ETickPriority::ETP_None;

	int mTickId = -1;

	bool mbActivate = true;

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

	__forceinline bool IsActivate() const
	{
		return mbActivate;
	}


	friend class WWorld;
};