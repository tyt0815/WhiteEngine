#pragma once
#include "Utility/Memory.h"

extern class WWorld* g_World;

namespace ETickGroup
{
	enum ETickGroup : unsigned int
	{
		ETG_PrePhysics = 0,
		ETG_PostPhysics,
		ETG_None
	};
}

namespace ETickPriority
{
	enum ETickPriority : unsigned int
	{
		ETP_Early = 0,
		ETP_Late,
		ETP_None
	};
}

class WObject : public std::enable_shared_from_this<WObject>
{
public:
	virtual void Tick(float DeltaSecond);

	void SetTickGroup(ETickGroup::ETickGroup TickGroup, ETickPriority::ETickPriority TickPriority);

	virtual void Destroy() = 0;

	virtual void Activate() = 0;

	virtual void Deactivate() = 0;

protected:
	virtual void OnDestroy();

	virtual void OnActivate();

	virtual void OnDeactivate();

private:
	ETickGroup::ETickGroup mTickGroup = ETickGroup::ETG_None;
	ETickPriority::ETickPriority mTickPriority = ETickPriority::ETP_None;

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