#pragma once
#include "Component/SplineComponent.h"
#include "GameFramework/Interface/CollisionGenerator.h"
#include "Physics/HitResult.h"
#include "Utility/Container.h"


class WSplineCollisionComponent : public WSplineComponent, public FCollisionGeneratorBase
{
	typedef WSplineComponent Super;
	struct FCapsuleCollider
	{
		WSceneComponent* Comp = nullptr;
		XMFLOAT3 PrevLocation;
		float Radius;
		float HalfHeight;
	};

	struct FBoundingBox
	{
		WSceneComponent* CenterComp = nullptr;
		XMFLOAT3 Extent;
		XMFLOAT3 PrevLocation;
	};

public:
	WSplineCollisionComponent();

	virtual void Tick(float DeltaSecond) override;

	virtual void BeginComponent() override;

	virtual void GenerateCollision() override;

protected:
	virtual void OnActivate() override;

	virtual void OnDeactivate() override;

public:
	TArray<FCapsuleCollider> mCapsuleCollider;

private:
	FBoundingBox mBoundingBox;

	bool mbUseBoundingBox = false;

	float mElapsedTime = 0;

	int mSegment = 1;

public:
	__forceinline void SetSegment(int Value)
	{
		mSegment = max(Value, 1);
	}

	__forceinline void SetBoundingBox(bool bUse)
	{
		mbUseBoundingBox = bUse;
	}
};

REGISTER_COMPONENT(WSplineCollisionComponent);