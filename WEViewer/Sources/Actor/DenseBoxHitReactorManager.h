#pragma once
#include "HitReactorManager.h"

class AButton;

class ADenseBoxHitReactorManager : public AHitReactorManager
{
	typedef AHitReactorManager Super;
public:
	virtual void BeginPlay() override;

protected:
	virtual void SpawnHitReactors() override;

private:
	XMINT3 mSize = XMINT3(1, 1, 1);

public:
	__forceinline void SetSize(const XMINT3& InSize)
	{
		mSize = InSize;
	}
};