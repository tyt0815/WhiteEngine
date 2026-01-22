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

struct FCurveView
{
	unsigned char* StartPtr;
	float* FramesPtr;
	FKeyframeData* KeyframeDatasPtr;
	int TotalKeyFrameNum;
};

class FObjectAnimDataAsset : public FAsset
{
public:
	virtual bool LoadAsset(const std::wstring& FilePath);

	const FCurveView* GetCurveInfoSafe(const std::string& ObjectName, const std::string& CurveName) const;

	void GetObjectNames(std::vector<std::string>& Names);
	
private:
	unsigned char* mCurvesStartPtr;

	std::vector<unsigned char> mRawBuffer;

	std::unordered_map<std::string, std::unordered_map<std::string, FCurveView>> mObjectCurveMap;

	float mFPS = 1;

	float mDuration = 0;

public:
	__forceinline const std::unordered_map<std::string, std::unordered_map<std::string, FCurveView>>& GetObjectCurveMap() const
	{
		return mObjectCurveMap;
	}

	__forceinline const FCurveView* GetCurveInfo(const std::string& ObjectName, const std::string& CurveName) const
	{
		return &mObjectCurveMap.at(ObjectName).at(CurveName);
	}

	__forceinline const std::unordered_map<std::string, FCurveView>* GetObjectCurves(const std::string& ObjectName)
	{
		return &mObjectCurveMap.at(ObjectName);
	}

	__forceinline float GetFPS() const
	{
		return mFPS;
	}

	__forceinline float GetFraneEnd() const
	{
		return mDuration;
	}
};