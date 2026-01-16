#pragma once
#include "SceneComponent.h"
#include "Physics/PhysicsBody.h"
#include "Physics/ObjectChannel.h"


class WPrimitiveComponent : public WSceneComponent
{
	typedef WSceneComponent Super;

protected:
	virtual void Update() override;

protected:
	virtual void UpdateConstantBufferIndex();

	virtual void UpdateProxies();

	size_t mMeshCBIndex;
};