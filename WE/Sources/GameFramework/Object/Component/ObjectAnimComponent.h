#pragma once
#include "SceneComponent.h"
#include <string>
#include <tinyxml2.h>
#include <DirectXMath.h>
#include <unordered_map>
#include <vector>

#include "Asset/ObjectAnimDataAsset.h"

class FCurveSampler
{
public:
	float SampleAnimDataByFrame(const float TargetFrame);
	

private:
	FCurveSampler() = default;

	const FCurveView* mCurveView = nullptr;
	int mLastIndex = 0;

	friend class FObjectAnimSampler;

public:
	__forceinline bool IsValid() const
	{
		return mCurveView != nullptr;
	}
};

class FObjectAnimSampler
{
public:
	FCurveSampler GetCurveSampler(const std::string CurveName);

	FTransform SampleTransform(float TargetFrame);

private:
	FObjectAnimSampler(const std::unordered_map<std::string, FCurveView>* InCurveViewMap);

	const std::unordered_map<std::string, FCurveView>* mCurveViewMap;

	FCurveSampler mLocationX;
	FCurveSampler mLocationY;
	FCurveSampler mLocationZ;
	FCurveSampler mRotationX;
	FCurveSampler mRotationY;
	FCurveSampler mRotationZ;
	FCurveSampler mScaleX;
	FCurveSampler mScaleY;
	FCurveSampler mScaleZ;

	friend class WObjectAnimComponent;
};

class WObjectAnimComponent : public WSceneComponent
{
	typedef WSceneComponent Super;


public:
	virtual void BeginComponent() override;

public:
	bool LoadKeyframesFromOADAsset(const std::wstring& AssetName);

	FObjectAnimSampler* GetObjectAnimSampler(std::string ObjectName);

	FTransform SampleAnimWorldTransformByFrame(FObjectAnimSampler* Sampler, float Frame);

	FTransform SampleAnimWorldTransformBySecond(FObjectAnimSampler* Sampler, float Second);

private:
	std::unordered_map<std::string, FObjectAnimSampler> mObjectAnimSamplerMap;

	float mFrameEnd = 0;

	float mFps = 0;

public:
	__forceinline float GetFrameEnd() const
	{
		return mFrameEnd;
	}

	// frame의 시작이 0
	__forceinline float GetDuration() const
	{
		return mFrameEnd / mFps;
	}

	// frame의 시작이 0
	__forceinline float SecondToFrame(float Second) const
	{
		return mFps * Second;
	}
};