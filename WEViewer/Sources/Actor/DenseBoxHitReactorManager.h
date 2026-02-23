#pragma once
#include "HitReactorManager.h"

class ADenseBoxHitReactorManager : public AHitReactorManager
{
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