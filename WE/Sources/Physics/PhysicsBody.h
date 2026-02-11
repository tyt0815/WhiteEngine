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
	void Create(JPH::BodyCreationSettings Settings);

	void UpdateShape(JPH::ShapeRefC Shape);

	void Activate();

	void Deactivate();

	void SetLocation(const XMFLOAT3& Location);

	void SetRotation(const XMFLOAT4& Quaternion);

	void SetLocationAndRotation(const XMFLOAT3& Location, const XMFLOAT4& Quaternion);

	void SetMotiontype(JPH::EMotionType MotionType);

	XMFLOAT3 GetLocation() const;

	XMFLOAT3 GetRotation() const;

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

	__forceinline JPH::BodyID GetBodyID() const
	{
		return mBody->GetID();
	}
};