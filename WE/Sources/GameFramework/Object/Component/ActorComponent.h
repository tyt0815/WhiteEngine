#pragma once

#include "DirectX/DXMath.h"

class AActor;
class WWorld;

class WActorComponent
{
public:
	WActorComponent() {};

	virtual ~WActorComponent() noexcept = default;

public:
	virtual void BeginComponent() {};

	virtual void SetOwner(AActor* Owner);

	virtual void TickComponent_PrePhysics(float DeltaTime) {};

	virtual void TickComponent_PostPhysics(float DeltaTime) {};

	WWorld* GetWorld() const;

private:
	AActor* mOwner = nullptr;

public:
	inline AActor* GetOwner() const
	{
		return mOwner;
	}
};