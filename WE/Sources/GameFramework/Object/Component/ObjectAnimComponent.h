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
	void ToControlPoint(
		const FKeyframe& Left, const FKeyframe& Right,
		XMVECTOR* P0, XMVECTOR* P1, XMVECTOR* P2, XMVECTOR* P3
	) const;

	bool LoadKeyframesFromOADAsset(const std::wstring& AssetName);

	float SampleAnimDataByFrame(FAnimData* AnimData, const float TargetFrame);

	float SampleAnimDataBySecond(FAnimData* AnimData, float Second);

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