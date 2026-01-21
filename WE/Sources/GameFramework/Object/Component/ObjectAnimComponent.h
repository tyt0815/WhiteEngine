#pragma once
#include "SceneComponent.h"
#include <string>
#include <tinyxml2.h>
#include <DirectXMath.h>
#include <unordered_map>
#include <vector>

class WObjectAnimComponent : public WSceneComponent
{
	typedef WSceneComponent Super;
	enum class EInterpolationType : int
	{
		EIT_Linear,
		EIT_Bezier,
		EIT_Constant,
		EIT_Undefined
	};

	struct FKeyframe
	{
		float Frame;
		float Value;
		EInterpolationType Interpolation;
		XMFLOAT2 RightHandle;
		XMFLOAT2 LeftHandle;
	};

public:
	virtual void BeginComponent() override;

public:
	bool LoadKeyframesFromXMLAsset(const std::wstring& Name);

	bool LoadKeyframesFromXML(tinyxml2::XMLDocument* Doc);

	bool LoadKeyframesFromXMLFile(const std::string& FilePath);

	bool LoadKeyframesFromZlibXMLFile(const std::string& FilePath);

	bool LoadKeyframesFromZlibKeyframeMap(const std::string& FilePath);

	bool LoadKeyframesFromLZ4KeyframeMap(const std::string& FilePath);

	void ToControlPoint(
		const FKeyframe& Left, const FKeyframe& Right,
		XMVECTOR* P0, XMVECTOR* P1, XMVECTOR* P2, XMVECTOR* P3
	) const;

	float InterpolateKeyframeByFrame(const std::vector<FKeyframe>& Keyframes, float Frame) const;

	float InterpolateKeyframeBySecond(const std::vector<FKeyframe>& Keyframes, float Second) const;

	FTransform SampleAnimLocalTransformByFrame(float Frame) const;

	FTransform SampleAnimLocalTransformBySecond(float Second) const;

	FTransform SampleAnimWorldTransformByFrame(float Frame);

	FTransform SampleAnimWorldTransformBySecond(float Second);

	float GetPropertyByFrame(const std::string& PropertyName, float Frame) const;

	float GetPropertyBySecond(const std::string& PropertyName, float Second) const;

private:
	bool LoadKeyframesFromBinary(unsigned char* Ptr);

	std::unordered_map<std::string, std::vector<FKeyframe>> mKeyframeMap;

	std::vector<FKeyframe>* mLocXKeyframes = nullptr;
	std::vector<FKeyframe>* mLocYKeyframes = nullptr;
	std::vector<FKeyframe>* mLocZKeyframes = nullptr;
	std::vector<FKeyframe>* mRotXKeyframes = nullptr;
	std::vector<FKeyframe>* mRotYKeyframes = nullptr;
	std::vector<FKeyframe>* mRotZKeyframes = nullptr;
	std::vector<FKeyframe>* mScaleXKeyframes = nullptr;
	std::vector<FKeyframe>* mScaleYKeyframes = nullptr;
	std::vector<FKeyframe>* mScaleZKeyframes = nullptr;

	float mFPS = 1;

	float mFrameEnd = 0;

public:
	__forceinline float GetLastFrame() const
	{
		return mFrameEnd;
	}

	// frame의 시작이 0
	__forceinline float GetLastSecond() const
	{
		return mFrameEnd / mFPS;
	}

	// frame의 시작이 0
	__forceinline float SecondToFrame(float Second) const
	{
		return mFPS * Second;
	}
};