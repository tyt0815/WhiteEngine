
#pragma once
#include "SceneComponent.h"
#include "Physics/PhysicsBody.h"
#include "Physics/ObjectChannel.h"
#include "Utility/Delegate.h"
#include "Utility/Memory.h"

class WPhysicsComponent;

DECLARE_DELEGATE_TwoParams(FComponentHitDelegate, WPhysicsComponent*, XMFLOAT3);
DECLARE_DELEGATE_TwoParams(FComponentBeginOverlapDelegate, WPhysicsComponent*, XMFLOAT3);
DECLARE_DELEGATE_OneParam(FComponentExitHitDelegate, WPhysicsComponent*);
DECLARE_DELEGATE_OneParam(FComponentEndOverlapDelegate, WPhysicsComponent*);

using EMotionType = JPH::EMotionType;

using EAllowedDOFs = JPH::EAllowedDOFs;

class WPhysicsComponent : public WSceneComponent
{
	typedef WSceneComponent Super;
public:
	WPhysicsComponent();

	virtual void BeginComponent() override;

protected:
	virtual void OnActivate() override;

	virtual void OnDeactivate() override;

	virtual void PostSetupAttachment() override;

public:
	void UpdateToPhysics();

	void UpdateFromPhysics();

	void ActivatePhysicBody();

	void DeactivatePhysicBody();

	void SetMotionType(EMotionType MotionType);

	void SetObjectChannel(EObjectChannel::EObjectChannel ObjectChannel);

	void AddImpulse(const XMFLOAT3& Impulse, const XMFLOAT3& Point);

	FComponentHitDelegate mOnHitDelegate;

	FComponentBeginOverlapDelegate mOnBeginOverlapDelegate;

	FComponentExitHitDelegate mOnExitHitDelegate;

	FComponentEndOverlapDelegate mOnEndOverlapDelegate;

	EAllowedDOFs mAllowedDOFs = EAllowedDOFs::All;

	float mGravityFactor = 1;

	float mFriction = 0.2f;

	float mMaxLinearVelocity = 500.0f;

protected:
	virtual JPH::ShapeRefC CreatePhysicsShape() = 0;

	void CreatePhysicsBody();

	EMotionType mMotionType = JPH::EMotionType::Dynamic;

	EObjectChannel::EObjectChannel mObjectChannel = EObjectChannel::EOC_WorldDynamic;

	bool mbGenerateOverlapEvent = false;

	bool mbPhysicSimulate = false;

	FTransform mLastPhysicsTransform;

private:
	TUniquePtr<FPhysicsBody> mBody;

	int mPhysicsCompQueueId = -1;

	float mMass = 1;

public:
	__forceinline void SetMass(float Value)
	{
		mMass = Value;
	}

	friend class WWorld;
};