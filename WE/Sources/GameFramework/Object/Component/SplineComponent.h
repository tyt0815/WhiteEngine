#pragma once

#include "SceneComponent.h"
#include <string>

class WSplineComponent : public WSceneComponent
{
	typedef WSceneComponent Super;

	struct FSplineNode
	{
		XMFLOAT3 ControlPoint;
		XMFLOAT3 LeftHandle;
		XMFLOAT3 RightHandle;
		float Property1;		// Blender의 weight;
		float Property2;		// Blender의 Radius;
	};
public:
	struct FSplineLUT
	{
		XMFLOAT3 Location;
		float Distance;
		XMFLOAT4 Quat;
		float Property1;
		float Property2;
	};
public:
	WSplineComponent();

	virtual void Tick(float Delta) override;

public:
	void AddSplineNode(const FSplineNode& Node);

	void LoadSplineFromAsset(const std::string& AssetName);

	// InputKey의 범위는 [0, n - 1] 여기서 n: SplineNode의 수
	XMFLOAT3 GetLocalLocationAtSplineInputKey(float InputKey);

	XMFLOAT3 GetLocalRotationAtSplineInputKey(float InputKey);

	FTransform GetLocalTransformAtSplineInputKey(float InputKey);

	FTransform GetWorldTransformAtSplineInputKey(float InputKey);

	XMFLOAT3 GetLocalLocationAtDistanceAlongSpline(float Distance);

	XMFLOAT3 GetLocalRotationAtDistanceAlongSpline(float Distance);

	XMFLOAT4 GetLocalQuaternionAtDistanceAlongSpline(float Distance);

	FTransform GetLocalTransformAtDistanceAlongSpline(float Distance);

	FTransform GetWorldTransformAtDistanceAlongSpline(float Distance);

	float GetCustomProperty1AtDistanceAlongSpline(float Distance);

	float GetCustomProperty2AtDistanceAlongSpline(float Distance);

	FSplineLUT GetSplineLUTAtDistanceAlongSpline(float Distance);
	
private:
	std::vector<FSplineNode> mSplineNodes;

	void SelectSplineNodesByInputKey(float InputKey, FSplineNode& LeftNode, FSplineNode& RightNode, float& t);

	void SelectBezierPointsByInputKey(float InputKey, XMVECTOR* P0, XMVECTOR* P1, XMVECTOR* P2, XMVECTOR* P3, float& t);

	struct FSampleParam
	{
		float t0;
		float t1;
		float Tolerance;
		XMVECTOR Pt0;
		XMVECTOR Pt1;
		XMVECTOR P0;
		XMVECTOR P1;
		XMVECTOR P2;
		XMVECTOR P3;
		float Property1_0;
		float Property1_1;
		float Property2_0;
		float Property2_1;
	};
	
	void AdaptiveSampleRecursive(const FSampleParam& Param, UINT Depth);

	std::vector<FSplineLUT> mSplineLUT;

public:
	__forceinline size_t GetControllPointNum() const
	{
		return mSplineNodes.size();
	}

	__forceinline float GetSplineLength() const
	{
		return mSplineLUT.size() > 0 ? mSplineLUT.back().Distance : 0;
	}

	__forceinline int GetSplineLUTNum() const
	{
		return (int)mSplineLUT.size();
	}

	__forceinline const FSplineLUT* GetSplineLUTAt(int i) const
	{
		return &mSplineLUT[i];
	}
};

REGISTER_COMPONENT(WSplineComponent);