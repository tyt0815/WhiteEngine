#pragma once

#include "SceneComponent.h"
#include <string>

class WSplineComponent : public WSceneComponent
{
	struct FSplineNode
	{
		XMFLOAT3 ControlPoint;
		XMFLOAT3 LeftHandle;
		XMFLOAT3 RightHandle;
		float Property1;		// Blender의 weight;
		float Property2;		// Blender의 Radius;
	};

public:
	void AddSplineNode(const FSplineNode& Node);

	void LoadSplineFromAsset(const std::wstring& AssetName);

	// InputKey의 범위는 [0, n - 1] 여기서 n: SplineNode의 수
	XMFLOAT3 GetLocalLocationAtSplineInputKey(float InputKey);

	XMFLOAT3 GetLocalRotationAtSplineInputKey(float InputKey);

	FTransform GetLocalTransformAtSplineInputKey(float InputKey);

	FTransform GetWorldTransformAtSplineInputKey(float InputKey);
	
private:
	std::vector<FSplineNode> mSplineNodes;

	void SelectSplineNodesByInputKey(float InputKey, FSplineNode& LeftNode, FSplineNode& RightNode, float& t);

	void SelectBezierPointsByInputKey(float InputKey, XMVECTOR* P0, XMVECTOR* P1, XMVECTOR* P2, XMVECTOR* P3, float& t);

public:
	__forceinline size_t GetControllPointNum() const
	{
		return mSplineNodes.size();
	}
};