#pragma once

#include "SceneComponent.h"

class WSplineComponent : public WSceneComponent
{
public:

private:
	struct FSplineNode
	{
		XMFLOAT3 ControlPoint;
		XMFLOAT3 LeftHandle;
		XMFLOAT3 RightHandle;
	};

	std::vector<FSplineNode> mSplineNodes;

	void AddSplineNode(const FSplineNode& Node);
};