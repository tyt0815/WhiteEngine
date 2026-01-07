#pragma once

#include "DirectX/DXMath.h"

class AActor;

class WActorComponent
{
public:
	WActorComponent() {};

	virtual ~WActorComponent() noexcept = default;

public:
	virtual void SetOwner(AActor* Owner);

	virtual void TickComponent(float DeltaTime) {};

private:
	AActor* mOwner = nullptr;

public:
	inline AActor* GetOwner() const
	{
		return mOwner;
	}
};