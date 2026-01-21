#pragma once

#include "DirectX/DXMath.h"
#include "Utility/Memory.h"

class AActor;
class WWorld;

class WActorComponent : public std::enable_shared_from_this<WActorComponent>
{
public:
	WActorComponent() {};

	virtual ~WActorComponent() noexcept = default;

public:
	virtual void BeginComponent() {};

	virtual void SetOwner(TWeakPtr<AActor> Owner);

	virtual void TickComponent_PrePhysics(float DeltaTime) {};

	virtual void TickComponent_PostPhysics(float DeltaTime) {};

	WWorld* GetWorld() const;

	TWeakPtr<AActor> mOwner;

protected:
	virtual void OnActivate() {};

	virtual void OnDeactivate() {};

public:
	inline TWeakPtr<AActor> GetOwner() const
	{
		return mOwner;
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
};