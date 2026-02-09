#pragma once
#include "SceneComponent.h"
#include <string>
#include <tinyxml2.h>
#include <DirectXMath.h>
#include <unordered_map>

#include "Asset/ObjectAnimDataAsset.h"
#include "Utility/Container.h"

class FCurveSampler
{
public:
	float SampleAnimDataByFrame(const float TargetFrame);

private:

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
	FObjectAnimSampler(const std::unordered_map<std::string, FCurveView>* InCurveViewMap);

	FCurveSampler GetCurveSampler(const std::string CurveName);

	FTransform SampleTransform(float TargetFrame);

	float SampleLocationX(float TargetFrame);

	float SampleLocationY(float TargetFrame);

	float SampleLocationZ(float TargetFrame);

	XMFLOAT3 SampleLocation(float TargetFrame);

private:

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

namespace ERootMotion
{
	enum Flags : uint16_t
	{
		None = 0,
		LocX = 1 << 0, LocY = 1 << 1, LocZ = 1 << 2,
		RotX = 1 << 3, RotY = 1 << 4, RotZ = 1 << 5,
		ScaleX = 1 << 6, ScaleY = 1 << 7, ScaleZ = 1 << 8,

		AllLoc = LocX | LocY | LocZ,
		AllRot = RotX | RotY | RotZ,
		AllScale = ScaleX | ScaleY | ScaleZ,
		All = AllLoc | AllRot | AllScale
	};
}

class WObjectAnimComponent : public WActorComponent
{
	typedef WActorComponent Super;

	struct FCurveBind
	{
		FCurveSampler CurveSampler;
		float* TargetPtr = nullptr;
		float BaseValue;
		bool bModifier;
	};

public:
	WObjectAnimComponent();

	virtual void BeginComponent() override;

	virtual void Tick(float DeltaTime) override;

public:
	bool LoadAnimation(const std::string& AssetName, const std::string& AnimName);

	void LoadAndPlay(const std::string& AssetName, const std::string& AnimName, float PlayRate, bool bLoop, uint16_t Flags);

	// @param CurveName Curve의 이름
	// @param TargetPtr 바인딩할 값의 포인터
	// @param bModifier 모디파이어로 작동할 것 인지. false일 경우 TargetPtr = CurveValue, true일 경우 BaseValue * CurveValue
	// @param BaseValue bModifier가 true일 경우 사용되는 옵션
	void BindCurve(const std::string& CurveName, float* TargetPtr, bool bModifier = false, float BaseValue = 0);

	void RemoveBoundCurveAt(int Index);

	int GetBoundCurveIndex(float* TargetPtr);

	FCurveBind* GetBoundCurve(float* TargetPtr);

	void Play(float PlayRate, bool bLoop, uint16_t Flags);

	void Stop();

	void SetTargetComponent(WSceneComponent* Comp);

private:
	TUniquePtr<FObjectAnimSampler> mSampler;

	TWeakPtr<WSceneComponent> mTarget;

	TArray<FCurveBind> mBoundCurves;

	float mFrameEnd = 0;
	float mFps = 0;

	float mCurrentTime = 0.0f;
	bool  mIsPlaying = false;
	bool  mLoop = false;
	float mPlayRate = 1.0f;
	uint16_t mRootMotionFlags;

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

REGISTER_COMPONENT(WObjectAnimComponent);