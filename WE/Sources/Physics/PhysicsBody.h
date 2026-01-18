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

	void AddBody(bool bActivate = true);

	void RemoveBody();

	void SetPosition(XMFLOAT3 Position);

	void SetMotiontype(JPH::EMotionType MotionType);

	void SetActivate(bool bActivate);

	FTransform GetTransform() const;

	void SetTransform(const FTransform& Transform);

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