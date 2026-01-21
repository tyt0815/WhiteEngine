#pragma once
#include "Asset.h"
#include <unordered_map>

enum class EInterpolationType : int
{
	EIT_Linear = 0,
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

struct FAnimData
{
	std::vector<FKeyframe> Keyframes;
	int LastIndex = 0;
};

class FObjectAnimDataAsset : public FAsset
{
public:
	virtual bool LoadAsset(const std::wstring& FilePath);

	std::vector<unsigned char> RawBuffer;

	std::unordered_map<std::string, FAnimData> KeyframeMap;

	float FPS = 1;

	float FrameEnd = 0;
};