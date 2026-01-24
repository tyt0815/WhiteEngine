#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include "DirectX/DXMath.h"
#include "Utility/Memory.h"

namespace Physics
{
	struct FUserData;
}

class FPhysicsBody
{
public:
	FPhysicsBody(class WPhysicsComponent* Owner) : mOwner(Owner) {};

	~FPhysicsBody();

public:
	void CreateBody(JPH::BodyCreationSettings Settings);

	void UpdateShape(JPH::ShapeRefC Shape);

	void Activate();

	void Deactivate();

	void SetPosition(XMFLOAT3 Position);

	void SetMotiontype(JPH::EMotionType MotionType);

	void SetActivate(bool bActivate);

	XMFLOAT3 GetLocation() const;

	XMFLOAT3 GetRotation() const;

	void SetTransform(const FTransform& Transform);

protected:


private:
	FPhysicsBody() = delete;

	JPH::Body* mBody = nullptr;

	WPhysicsComponent* mOwner;

	Physics::FUserData* mUserData;

public:
	__forceinline bool IsValid() const
	{
		return mBody != nullptr;
	}
};