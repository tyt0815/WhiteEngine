#pragma once
#include "Component/SplineComponent.h"
#include "Physics/HitResult.h"
#include "Utility/Container.h"
#include "Utility/Delegate.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCollision, const FHitResult&);

class WSplineCollisionComponent : public WSplineComponent
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

public:
	void GenerateCapsuleCollision(int Segment, bool bUseBoundingBox);

	TArray<FCapsuleCollider> mCapsuleCollider;

	FOnCollision OnCollision;

private:
	FBoundingBox mBoundingBox;
	bool mbUseBoundingBox = false;

	float mElapsedTime = 0;
};

REGISTER_COMPONENT(WSplineCollisionComponent);