#pragma once
#include "SceneComponent.h"
#include <string>
#include <tinyxml2.h>
#include <DirectXMath.h>
#include <unordered_map>
#include <vector>

class WObjectAnimComponent : public WSceneComponent
{
	enum class EInterpolationType
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
	void LoadXML(const std::wstring& Name);

	void ToControlPoint(
		const FKeyframe& Left, const FKeyframe& Right,
		XMVECTOR* P0, XMVECTOR* P1, XMVECTOR* P2, XMVECTOR* P3
	) const;

	float InterpolateKeyframeByFrame(const std::vector<FKeyframe>& Keyframes, float Frame) const;

	float InterpolateKeyframeBySecond(const std::vector<FKeyframe>& Keyframes, float Second) const;

	FTransform GetKeyframeLocalTransformByFrame(float Frame) const;

	FTransform GetKeyframeLocalTransformBySecond(float Second) const;

	FTransform GetKeyframeWorldTransformByFrame(float Frame);

	FTransform GetKeyframeWorldTransformBySecond(float Second);

	float GetPropertyByFrame(const std::string& PropertyName, float Frame) const;

	float GetPropertyBySecond(const std::string& PropertyName, float Second) const;

private:
	std::unordered_map<std::string, std::vector<FKeyframe>> mKeyframeMap;

	float mFPS = 1;

	tinyxml2::XMLDocument* Doc;

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