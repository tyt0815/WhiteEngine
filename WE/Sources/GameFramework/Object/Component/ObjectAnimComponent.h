#pragma once
#include "SceneComponent.h"
#include <string>
#include <tinyxml2.h>
#include <DirectXMath.h>
#include <unordered_map>
#include <vector>

#include "Asset/ObjectAnimDataAsset.h"

class WObjectAnimComponent : public WSceneComponent
{
	typedef WSceneComponent Super;


public:
	virtual void BeginComponent() override;

public:
	bool LoadKeyframesFromOADAsset(const std::wstring& AssetName);

	float SampleAnimDataByFrame(FAnimData* AnimData, int& LastIndex, const float TargetFrame);

	float SampleAnimDataBySecond(FAnimData* AnimData, int& LastIndex, float Second);

	FTransform SampleAnimLocalTransformByFrame(float Frame);

	FTransform SampleAnimLocalTransformBySecond(float Second);

	FTransform SampleAnimWorldTransformByFrame(float Frame);

	FTransform SampleAnimWorldTransformBySecond(float Second);

private:

	FObjectAnimDataAsset* mObjectAnimData;

	FAnimData* mLocXKeyframes = nullptr;
	FAnimData* mLocYKeyframes = nullptr;
	FAnimData* mLocZKeyframes = nullptr;
	FAnimData* mRotXKeyframes = nullptr;
	FAnimData* mRotYKeyframes = nullptr;
	FAnimData* mRotZKeyframes = nullptr;
	FAnimData* mScaleXKeyframes = nullptr;
	FAnimData* mScaleYKeyframes = nullptr;
	FAnimData* mScaleZKeyframes = nullptr;

	int mLocXKeyframeLastIndex = 0;
	int mLocYKeyframeLastIndex = 0;
	int mLocZKeyframeLastIndex = 0;
	int mRotXKeyframeLastIndex = 0;
	int mRotYKeyframeLastIndex = 0;
	int mRotZKeyframeLastIndex = 0;
	int mScaleXKeyframeLastIndex = 0;
	int mScaleYKeyframeLastIndex = 0;
	int mScaleZKeyframeLastIndex = 0;

public:
	__forceinline float GetLastFrame() const
	{
		return mObjectAnimData ? mObjectAnimData->FrameEnd : 0;
	}

	// frame의 시작이 0
	__forceinline float GetLastSecond() const
	{
		return mObjectAnimData ? mObjectAnimData->FrameEnd / mObjectAnimData->FPS : 0;
	}

	// frame의 시작이 0
	__forceinline float SecondToFrame(float Second) const
	{
		return mObjectAnimData ? mObjectAnimData->FPS * Second : 0;
	}

	//__forceinline const FAnimData* GetAnimData(const std::string& Name) const
	//{
	//	return mObjectAnimData ? (mObjectAnimData->KeyframeMap->count(Name) ? &mObjectAnimData->KeyframeMap->at(Name) : nullptr) : nullptr;
	//}
};