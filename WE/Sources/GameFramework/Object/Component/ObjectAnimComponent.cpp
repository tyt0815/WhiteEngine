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
		if (mObjectAnimData->KeyframeMap.count("LocationX")) { mLocXKeyframes = &(mObjectAnimData->KeyframeMap)["LocationX"]; }
		if (mObjectAnimData->KeyframeMap.count("LocationY")) { mLocYKeyframes = &(mObjectAnimData->KeyframeMap)["LocationY"]; }
		if (mObjectAnimData->KeyframeMap.count("LocationZ")) { mLocZKeyframes = &(mObjectAnimData->KeyframeMap)["LocationZ"]; }
		if (mObjectAnimData->KeyframeMap.count("RotationX")) { mRotXKeyframes = &(mObjectAnimData->KeyframeMap)["RotationX"]; }
		if (mObjectAnimData->KeyframeMap.count("RotationY")) { mRotYKeyframes = &(mObjectAnimData->KeyframeMap)["RotationY"]; }
		if (mObjectAnimData->KeyframeMap.count("RotationZ")) { mRotZKeyframes = &(mObjectAnimData->KeyframeMap)["RotationZ"]; }
		if (mObjectAnimData->KeyframeMap.count("ScaleX")) { mScaleXKeyframes =	&(mObjectAnimData->KeyframeMap)["ScaleX"]; }
		if (mObjectAnimData->KeyframeMap.count("ScaleY")) { mScaleYKeyframes =	&(mObjectAnimData->KeyframeMap)["ScaleY"]; }
		if (mObjectAnimData->KeyframeMap.count("ScaleZ")) { mScaleZKeyframes =	&(mObjectAnimData->KeyframeMap)["ScaleZ"]; }
	}
}

bool WObjectAnimComponent::LoadKeyframesFromOADAsset(const std::wstring& AssetName)
{
	mObjectAnimData = FAssetManager::GetAsset<FObjectAnimDataAsset>(L"OAD_Large");
	return mObjectAnimData != nullptr;
}

float WObjectAnimComponent::SampleAnimDataByFrame(FAnimData* AnimData, int& LastIndex, const float TargetFrame)
{
	const std::vector<float>& Frames = AnimData->Frames;
	const std::vector<FKeyframeData>& Keyframes = AnimData->Keyframes;
	const int NumKeys = static_cast<int>(Keyframes.size());

	if (NumKeys == 0) return 0.0f;
	if (NumKeys == 1) return Keyframes[0].Value;

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
		auto it = std::lower_bound(Frames.begin(), Frames.end(), TargetFrame);
		i = static_cast<int>(std::distance(Frames.begin(), it));
	}

	// 3. 인덱스 범위 클램핑 및 LastIndex 갱신
	// i가 0이면 타겟이 첫 키보다 앞에 있음, i가 NumKeys면 마지막 키보다 뒤에 있음
	LastIndex = std::clamp(i, 1, NumKeys - 1);

	// 4. 경계값 처리
	if (TargetFrame <= Frames[0]) return Keyframes[0].Value;
	if (TargetFrame >= Frames[NumKeys - 1]) return Keyframes[NumKeys - 1].Value;

	// 5. 보간 수행 (이제 i는 항상 1 ~ NumKeys-1 사이임이 보장됨)
	float LeftFrame = Frames[LastIndex - 1];
	float RightFrame = Frames[LastIndex];
	const FKeyframeData& Left = Keyframes[LastIndex - 1];
	const FKeyframeData& Right = Keyframes[LastIndex];
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

float WObjectAnimComponent::SampleAnimDataBySecond(FAnimData* AnimData, int& LastIndex, float Second)
{
	return SampleAnimDataByFrame(AnimData, LastIndex, SecondToFrame(Second));
}

FTransform WObjectAnimComponent::SampleAnimLocalTransformByFrame(float Frame)
{
	FTransform Transform;

	// 1. Location (사용자 정의 이름: LocationX, LocationY, LocationZ)
	if (mLocXKeyframes) Transform.Translation.x = SampleAnimDataByFrame(mLocXKeyframes, mLocXKeyframeLastIndex, Frame);
	if (mLocYKeyframes) Transform.Translation.y = SampleAnimDataByFrame(mLocYKeyframes, mLocYKeyframeLastIndex, Frame);
	if (mLocZKeyframes) Transform.Translation.z = SampleAnimDataByFrame(mLocZKeyframes, mLocZKeyframeLastIndex, Frame);

	// 2. Rotation (사용자 정의 이름: RotationX, RotationY, RotationZ, RotationW)
	if (mRotXKeyframes) Transform.Rotation.x = SampleAnimDataByFrame(mRotXKeyframes, mRotXKeyframeLastIndex, Frame);
	if (mRotYKeyframes) Transform.Rotation.y = SampleAnimDataByFrame(mRotYKeyframes, mRotYKeyframeLastIndex, Frame);
	if (mRotZKeyframes) Transform.Rotation.z = SampleAnimDataByFrame(mRotZKeyframes, mRotZKeyframeLastIndex, Frame);

	// 3. Scale (사용자 정의 이름: ScaleX, ScaleY, ScaleZ)
	Transform.Scale = { 1.0f, 1.0f, 1.0f }; // 기본값 설정
	if (mScaleXKeyframes) Transform.Scale.x = SampleAnimDataByFrame(mScaleXKeyframes, mScaleXKeyframeLastIndex, Frame);
	if (mScaleYKeyframes) Transform.Scale.y = SampleAnimDataByFrame(mScaleYKeyframes, mScaleYKeyframeLastIndex, Frame);
	if (mScaleZKeyframes) Transform.Scale.z = SampleAnimDataByFrame(mScaleZKeyframes, mScaleZKeyframeLastIndex, Frame);

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
