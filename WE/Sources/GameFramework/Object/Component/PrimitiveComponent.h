#pragma once
#include "SceneComponent.h"
#include "Physics/PhysicsBody.h"
#include "Physics/ObjectChannel.h"
#include "Utility/Delegate.h"

DECLARE_DELEGATE_TwoParams(FComponentHitDelegate, class WPrimitiveComponent*, XMFLOAT3);
DECLARE_DELEGATE_TwoParams(FComponentBeginOverlapDelegate, class WPrimitiveComponent*, XMFLOAT3);

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

class WPrimitiveComponent : public WSceneComponent
{
	typedef WSceneComponent Super;
public:
	WPrimitiveComponent();

	virtual void BeginComponent() override;

protected:
	virtual void Update() override;

public:
	void UpdateToPhysics();

	void UpdateFromPhysics();

	void ActivatePhysicBody();

	void SetMotionType(EMotionType MotionType);

	void SetObjectChannel(EObjectChannel::EObjectChannel ObjectChannel);

	void GenerateOverlapEvent();

	FComponentHitDelegate mOnHitDelegate;

	FComponentBeginOverlapDelegate mOnBeginOverlapDelegate;

protected:
	virtual JPH::BodyCreationSettings CreatePhysicsBodySettings() = 0;

	void CreatePhysicsBody();

	virtual void UpdateConstantBufferIndex();

	virtual void UpdateProxies();

	size_t mMeshCBIndex;

	// Physics
	std::unique_ptr<FPhysicsBody> mBody;

	EMotionType mMotionType = JPH::EMotionType::Dynamic;

	EObjectChannel::EObjectChannel mObjectChannel = EObjectChannel::EOC_Moving;

	bool mbGenerateOverlapEvent = false;

	bool mbPhysicSimulate = false;
};