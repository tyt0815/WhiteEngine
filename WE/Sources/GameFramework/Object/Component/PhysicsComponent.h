#pragma once
#include "SceneComponent.h"
#include "Physics/PhysicsBody.h"
#include "Utility/Delegate.h"

DECLARE_DELEGATE_TwoParams(FComponentHitDelegate, class WPhysicsComponent*, XMFLOAT3);
DECLARE_DELEGATE_TwoParams(FComponentBeginOverlapDelegate, class WPhysicsComponent*, XMFLOAT3);

using EMotionType = JPH::EMotionType;

namespace EObjectChannel
{
	enum EObjectChannel : JPH::ObjectLayer
	{
		// ObjectChannel의 값들과 일치해야함
		EOC_NonMoving = 0,
		EOC_Moving,
		EOC_ChannelNum
	};
}

class WPhysicsComponent : public WSceneComponent
{
	typedef WSceneComponent Super;
public:
	WPhysicsComponent();

	virtual void BeginComponent() override;

public:
	void UpdateToPhysics();

	void UpdateFromPhysics();

	void ActivatePhysicBody();

	void GenerateOverlapEvent();

	void SetMotionType(EMotionType MotionType);

	void SetObjectChannel(EObjectChannel::EObjectChannel ObjectChannel);

	FComponentHitDelegate mOnHitDelegate;

	FComponentBeginOverlapDelegate mOnBeginOverlapDelegate;

protected:
	virtual JPH::BodyCreationSettings CreatePhysicsBodySettings() = 0;

	void CreatePhysicsBody();

	EMotionType mMotionType = JPH::EMotionType::Dynamic;

	EObjectChannel::EObjectChannel mObjectChannel = EObjectChannel::EOC_Moving;

	bool mbGenerateOverlapEvent = false;

	bool mbPhysicSimulate = false;

private:
	std::unique_ptr<FPhysicsBody> mBody;
};