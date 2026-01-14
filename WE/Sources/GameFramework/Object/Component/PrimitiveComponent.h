#pragma once
#include "SceneComponent.h"
#include "Physics/PhysicsCore.h"

class WPrimitiveComponent : public WSceneComponent
{
	typedef WSceneComponent Super;
public:
	WPrimitiveComponent();

	virtual void BeginPlay() override;

protected:
	virtual void Update() override;

public:
	void UpdatePhysics();

	void UpdatePhysicsTransform();

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
	void ActivatePhysicBody();

public:
};