#pragma once
#include "Component/SplineComponent.h"
#include "Utility/Container.h"

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

public:
	void GenerateCapsuleCollision(int Segment, bool bUseBoundingBox);

	TArray<FCapsuleCollider> mCapsuleCollider;

private:
	FBoundingBox mBoundingBox;
	bool mbUseBoundingBox = false;

	float mElapsedTime = 0;
};

REGISTER_COMPONENT(WSplineCollisionComponent);