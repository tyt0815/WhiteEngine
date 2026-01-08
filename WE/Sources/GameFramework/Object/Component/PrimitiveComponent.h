#pragma once
#include "SceneComponent.h"

class WPrimitiveComponent : public WSceneComponent
{
	typedef WSceneComponent Super;
public:
	WPrimitiveComponent();

protected:
	virtual void Update() override;

	virtual void UpdateConstantBufferIndex();

	virtual void UpdateProxies();

	size_t mMeshCBIndex;

private:

public:
};