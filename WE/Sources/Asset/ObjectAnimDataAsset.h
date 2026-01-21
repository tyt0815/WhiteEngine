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
	unsigned char* mCurvesStartPtr;

	std::vector<unsigned char> mRawBuffer;

	std::unordered_map<std::string, FCurveInfo> mCurveInfoMap;

	float mFPS = 1;

	float mFrameEnd = 0;

public:
	__forceinline const std::unordered_map<std::string, FCurveInfo>& GetCurveInfoMap() const
	{
		return mCurveInfoMap;
	}

	__forceinline const FCurveInfo* GetCurveInfoSafe(const std::string& CurveName) const
	{
		return mCurveInfoMap.count(CurveName) ? &mCurveInfoMap.at(CurveName) : nullptr;
	}

	__forceinline const FCurveInfo* GetCurveInfo(const std::string& CurveName) const
	{
		return &mCurveInfoMap.at(CurveName);
	}

	__forceinline float GetFPS() const
	{
		return mFPS;
	}

	__forceinline float GetFraneEnd() const
	{
		return mFrameEnd;
	}
};