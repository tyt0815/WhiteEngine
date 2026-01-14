#pragma once
#include "SceneComponent.h"
#include "Physics/PhysicsCore.h"

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

	FDelegate mOnHitDelegate;

	FDelegate mOnBeginOverlapDelegate;

protected:
	virtual void CreatePhysicsBody() = 0;

	virtual void UpdateConstantBufferIndex();

	virtual void UpdateProxies();

	size_t mMeshCBIndex;

	// Physics
	std::unique_ptr<FBody> mBody;

	EObjectType mObjectType = EObjectType::EOT_Dynamic;

	bool mbPhysicSimulate = false;

private:
	void OnComponentHit_Internal();

	void OnComponentBeginOverlap_Internal();

public:
	__forceinline void SetObjectType(EObjectType Type)
	{
		mObjectType = Type;
	}

};