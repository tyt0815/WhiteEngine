#pragma once
#include "SceneComponent.h"
#include <string>
#include <tinyxml2.h>
#include <DirectXMath.h>
#include <unordered_map>
#include <vector>

#include "Asset/ObjectAnimDataAsset.h"

struct FAnimData
{
	const FCurveInfo* CurveInfo;
	int LastIndex = 0;
};

class WObjectAnimComponent : public WSceneComponent
{
	typedef WSceneComponent Super;


public:
	virtual void BeginComponent() override;

public:
	bool LoadKeyframesFromOADAsset(const std::wstring& AssetName);

	float SampleAnimDataByFrame(FAnimData& AnimData, const float TargetFrame);

	float SampleAnimDataBySecond(FAnimData& AnimData, float Second);

	FTransform SampleAnimLocalTransformByFrame(float Frame);

	FTransform SampleAnimLocalTransformBySecond(float Second);

	FTransform SampleAnimWorldTransformByFrame(float Frame);

	FTransform SampleAnimWorldTransformBySecond(float Second);

private:
	FObjectAnimDataAsset* mObjectAnimData;

	FAnimData mLocXKeyframes;
	FAnimData mLocYKeyframes;
	FAnimData mLocZKeyframes;
	FAnimData mRotXKeyframes;
	FAnimData mRotYKeyframes;
	FAnimData mRotZKeyframes;
	FAnimData mScaleXKeyframes;
	FAnimData mScaleYKeyframes;
	FAnimData mScaleZKeyframes;

	float mLastFrame = 0;

	float mFps = 0;

public:
	__forceinline float GetLastFrame() const
	{
		return mLastFrame;
	}

	// frame의 시작이 0
	__forceinline float GetLastSecond() const
	{
		return mLastFrame / mFps;
	}

	// frame의 시작이 0
	__forceinline float SecondToFrame(float Second) const
	{
		return mFps * Second;
	}
};