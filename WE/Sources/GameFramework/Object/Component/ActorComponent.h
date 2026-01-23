#pragma once

#include "DirectX/DXMath.h"
#include "Utility/Memory.h"
#include "TickGroup.h"

class AActor;
class WWorld;

extern WWorld* g_World;

class WActorComponent : public std::enable_shared_from_this<WActorComponent>
{
public:
	WActorComponent() {};

	virtual ~WActorComponent() noexcept = default;

public:
	virtual void BeginComponent();

	virtual void SetOwner(TWeakPtr<AActor> Owner);

	virtual void TickComponent(float Delta) {};

	virtual void OnDestroy();

	TWeakPtr<AActor> mOwner;

protected:
	virtual void OnActivate();

	virtual void OnDeactivate();

	ETickGroup::ETickGroup mTickGroup = ETickGroup::ETG_None;

	int mTickQueueId = -1;

public:
	inline TWeakPtr<AActor> GetOwner() const
	{
		return mOwner;
	}

	__forceinline WWorld* GetWorld() const
	{
		return g_World;
	}
	
	template<typename T>
	__forceinline TWeakPtr<T> GetWeakPtr()
	{
		return Cast<T>(shared_from_this());
	}

	__forceinline TWeakPtr<WActorComponent> GetWeakPtr()
	{
		return GetWeakPtr<WActorComponent>();
	}

	friend class AActor;
	friend class WWorld;
};