#include "ObjectAnimComponent.h"
#include "Asset/AssetManager.h"
#include "Asset/XMLDataAsset.h"
#include "Asset/AssetLoader.h"


using namespace tinyxml2;

void WObjectAnimComponent::BeginComponent()
{
	Super::BeginComponent();
	
	if (mObjectAnimData)
	{
		auto& CurvInfoMap = mObjectAnimData->GetCurveInfoMap();
		if (CurvInfoMap.count("LocationX")) { mLocXKeyframes.CurveInfo	= mObjectAnimData->GetCurveInfo("LocationX"); }
		if (CurvInfoMap.count("LocationY")) { mLocYKeyframes.CurveInfo	= mObjectAnimData->GetCurveInfo("LocationY"); }
		if (CurvInfoMap.count("LocationZ")) { mLocZKeyframes.CurveInfo	= mObjectAnimData->GetCurveInfo("LocationZ"); }
		if (CurvInfoMap.count("RotationX")) { mRotXKeyframes.CurveInfo	= mObjectAnimData->GetCurveInfo("RotationX"); }
		if (CurvInfoMap.count("RotationY")) { mRotYKeyframes.CurveInfo	= mObjectAnimData->GetCurveInfo("RotationY"); }
		if (CurvInfoMap.count("RotationZ")) { mRotZKeyframes.CurveInfo	= mObjectAnimData->GetCurveInfo("RotationZ"); }
		if (CurvInfoMap.count("ScaleX")) { mScaleXKeyframes.CurveInfo	= mObjectAnimData->GetCurveInfo("ScaleX"); }
		if (CurvInfoMap.count("ScaleY")) { mScaleYKeyframes.CurveInfo	= mObjectAnimData->GetCurveInfo("ScaleY"); }
		if (CurvInfoMap.count("ScaleZ")) { mScaleZKeyframes.CurveInfo	= mObjectAnimData->GetCurveInfo("ScaleZ"); }
	}
}

bool WObjectAnimComponent::LoadKeyframesFromOADAsset(const std::wstring& AssetName)
{
	mObjectAnimData = FAssetManager::GetAsset<FObjectAnimDataAsset>(L"OAD_Large");
	mFps = mObjectAnimData->GetFPS();
	mFrameEnd = mObjectAnimData->GetFraneEnd();
	return mObjectAnimData != nullptr;
}

float WObjectAnimComponent::SampleAnimDataByFrame(FAnimData& AnimData, const float TargetFrame)
{
	const FCurveInfo* CurveInfo = AnimData.CurveInfo;
	int& LastIndex = AnimData.LastIndex;
	const float* Frames = CurveInfo->FramesPtr;
	const FKeyframeData* KeyframeDatas = CurveInfo->KeyframeDatasPtr;
	const int NumKeys = CurveInfo->TotalKeyFrameNum;

	if (NumKeys == 0) return 0.0f;
	if (NumKeys == 1) return KeyframeDatas[0].Value;

	// 1. 선형 탐색 시도 (LastIndex 기준 앞/뒤 10개)
	int i = LastIndex;
	const int SearchDir = Frames[i] <= TargetFrame ? 1 : -1;
	bool bFound = false;

	for (int Count = 0; Count < 10 && i >= 0 && i < NumKeys; ++Count, i += SearchDir)
	{
		// TargetFrame을 우측에 둔 구간 [i-1, i]를 찾음
		if (i > 0 && Frames[i - 1] <= TargetFrame && TargetFrame <= Frames[i])
		{
			bFound = true;
			break;
		}
	}

	// 2. 못 찾았다면 이분 탐색 (TargetFrame보다 크거나 같은 첫 번째 키를 찾음)
	if (!bFound)
	{
		const float* it = std::lower_bound(Frames, Frames + NumKeys, TargetFrame);
		i = static_cast<int>(std::distance(Frames, it));
	}

	// 3. 인덱스 범위 클램핑 및 LastIndex 갱신
	// i가 0이면 타겟이 첫 키보다 앞에 있음, i가 NumKeys면 마지막 키보다 뒤에 있음
	LastIndex = std::clamp(i, 1, NumKeys - 1);

	// 4. 경계값 처리
	if (TargetFrame <= Frames[0]) return KeyframeDatas[0].Value;
	if (TargetFrame >= Frames[NumKeys - 1]) return KeyframeDatas[NumKeys - 1].Value;

	// 5. 보간 수행 (이제 i는 항상 1 ~ NumKeys-1 사이임이 보장됨)
	float LeftFrame = Frames[LastIndex - 1];
	float RightFrame = Frames[LastIndex];
	const FKeyframeData& Left = KeyframeDatas[LastIndex - 1];
	const FKeyframeData& Right = KeyframeDatas[LastIndex];
	assert(TargetFrame >= LeftFrame);
	assert(TargetFrame <= RightFrame);
	assert(LeftFrame < RightFrame);
	float Alpha = (TargetFrame - LeftFrame) / (RightFrame - LeftFrame);

	float Value;
	switch (Left.Interpolation)
	{
	case EInterpolationType::EIT_Linear:
		Value = FDXMath::Lerp(Left.Value, Right.Value, Alpha);
		break;
	case EInterpolationType::EIT_Bezier:
	{
		XMFLOAT2 Point0 = { LeftFrame, Left.Value };
		const XMFLOAT2& Point1 = Left.RightHandle;
		const XMFLOAT2& Point2 = Right.LeftHandle;
		XMFLOAT2 Point3 = { RightFrame, Right.Value };
		XMVECTOR P0 = XMLoadFloat2(&Point0);
		XMVECTOR P1 = XMLoadFloat2(&Point1);
		XMVECTOR P2 = XMLoadFloat2(&Point2);
		XMVECTOR P3 = XMLoadFloat2(&Point3);

		Value = XMVectorGetY(FDXMath::CalculateCubicBezier(P0, P1, P2, P3, Alpha));
		break;
	}
	case EInterpolationType::EIT_Constant:
		Value = Left.Value;
		break;
	default:
		Value = 0;
		break;
	}

	return Value;
}

float WObjectAnimComponent::SampleAnimDataBySecond(FAnimData& AnimData, float Second)
{
	return SampleAnimDataByFrame(AnimData, SecondToFrame(Second));
}

FTransform WObjectAnimComponent::SampleAnimLocalTransformByFrame(float Frame)
{
	FTransform Transform;

	// 1. Location (사용자 정의 이름: LocationX, LocationY, LocationZ)
	if (mLocXKeyframes.CurveInfo) Transform.Translation.x = SampleAnimDataByFrame(mLocXKeyframes, Frame);
	if (mLocYKeyframes.CurveInfo) Transform.Translation.y = SampleAnimDataByFrame(mLocYKeyframes, Frame);
	if (mLocZKeyframes.CurveInfo) Transform.Translation.z = SampleAnimDataByFrame(mLocZKeyframes, Frame);

	// 2. Rotation (사용자 정의 이름: RotationX, RotationY, RotationZ, RotationW)
	if (mRotXKeyframes.CurveInfo) Transform.Rotation.x = SampleAnimDataByFrame(mRotXKeyframes, Frame);
	if (mRotYKeyframes.CurveInfo) Transform.Rotation.y = SampleAnimDataByFrame(mRotYKeyframes, Frame);
	if (mRotZKeyframes.CurveInfo) Transform.Rotation.z = SampleAnimDataByFrame(mRotZKeyframes, Frame);

	// 3. Scale (사용자 정의 이름: ScaleX, ScaleY, ScaleZ)
	if (mScaleXKeyframes.CurveInfo) Transform.Scale.x = SampleAnimDataByFrame(mScaleXKeyframes, Frame);
	if (mScaleYKeyframes.CurveInfo) Transform.Scale.y = SampleAnimDataByFrame(mScaleYKeyframes, Frame);
	if (mScaleZKeyframes.CurveInfo) Transform.Scale.z = SampleAnimDataByFrame(mScaleZKeyframes, Frame);

	return Transform;
}

FTransform WObjectAnimComponent::SampleAnimLocalTransformBySecond(float Second)
{
	return SampleAnimLocalTransformByFrame(SecondToFrame(Second));
}

FTransform WObjectAnimComponent::SampleAnimWorldTransformByFrame(float Frame)
{
	XMMATRIX CM = GetWorldMatrix();
	XMMATRIX M = SampleAnimLocalTransformByFrame(Frame).GetTransformMatrix();

	FTransform Transform;
	Transform.SetByTransformMatrix(M * CM);

	return Transform;
}

FTransform WObjectAnimComponent::SampleAnimWorldTransformBySecond(float Second)
{
	return SampleAnimWorldTransformByFrame(SecondToFrame(Second));
}
