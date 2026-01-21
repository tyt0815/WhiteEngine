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

struct FKeyframeData
{
	float Value;
	EInterpolationType Interpolation;
	XMFLOAT2 RightHandle;
	XMFLOAT2 LeftHandle;
};

struct FCurveInfo
{
	unsigned char* StartPtr;
	int TotalKeyFrameNum;
	float* FramesPtr;
	FKeyframeData* KeyframeDatasPtr;
};

class FObjectAnimDataAsset : public FAsset
{
public:
	virtual bool LoadAsset(const std::wstring& FilePath);
	
private:
	unsigned char* CurvesStartPtr;

	std::vector<unsigned char> RawBuffer;

	std::unordered_map<std::string, FCurveInfo> CurveInfoMap;

	float FPS = 1;

	float FrameEnd = 0;

	int TotalCurveNum = 0;

public:
	__forceinline const std::unordered_map<std::string, FCurveInfo>& GetCurveInfoMap() const
	{
		return CurveInfoMap;
	}

	__forceinline const FCurveInfo* GetCurveInfoSafe(const std::string& CurveName) const
	{
		return CurveInfoMap.count(CurveName) ? &CurveInfoMap.at(CurveName) : nullptr;
	}

	__forceinline const FCurveInfo* GetCurveInfo(const std::string& CurveName) const
	{
		return &CurveInfoMap.at(CurveName);
	}

	__forceinline float GetFPS() const
	{
		return FPS;
	}

	__forceinline float GetFraneEnd() const
	{
		return FrameEnd;
	}
};