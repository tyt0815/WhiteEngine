#pragma once
#include "GameFramework/Object/Object.h"

class WActorComponent : public WObject
{
public:
	virtual void SetOwner(AActor* Owner);
private:
	AActor* mOwner = nullptr;

public:
	inline AActor* GetOwner() const
	{
		return mOwner;
	}
};