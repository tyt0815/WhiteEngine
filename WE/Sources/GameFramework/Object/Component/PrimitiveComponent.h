#pragma once
#include "SceneComponent.h"

class WPrimitiveComponent : public WSceneComponent
{
	typedef WSceneComponent Super;
public:
	WPrimitiveComponent();

protected:
	virtual void Update() override;
	std::uint64_t mPrimitiveCBIndex;

private:

public:
};