#include "ObjectAnimComponent.h"
#include "Asset/AssetManager.h"
#include "Asset/XMLDataAsset.h"
#include "Asset/AssetLoader.h"


using namespace tinyxml2;


float FCurveSampler::SampleAnimDataByFrame(const float TargetFrame)
{
	if (mCurveView == nullptr)
	{
		return 0.0f;
	}

	const float* Frames = mCurveView->FramesPtr;
	const FKeyframeData* KeyframeDatas = mCurveView->KeyframeDatasPtr;
	const int NumKeys = mCurveView->TotalKeyFrameNum;

	if (NumKeys == 0) return 0.0f;
	if (NumKeys == 1) return KeyframeDatas[0].Value;

	// 1. 선형 탐색 시도 (LastIndex 기준 앞/뒤 10개)
	int i = mLastIndex;
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
	mLastIndex = std::clamp(i, 1, NumKeys - 1);

	// 4. 경계값 처리
	if (TargetFrame <= Frames[0]) return KeyframeDatas[0].Value;
	if (TargetFrame >= Frames[NumKeys - 1]) return KeyframeDatas[NumKeys - 1].Value;

	// 5. 보간 수행 (이제 i는 항상 1 ~ NumKeys-1 사이임이 보장됨)
	float LeftFrame = Frames[mLastIndex - 1];
	float RightFrame = Frames[mLastIndex];
	const FKeyframeData& Left = KeyframeDatas[mLastIndex - 1];
	const FKeyframeData& Right = KeyframeDatas[mLastIndex];
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

FCurveSampler FObjectAnimSampler::GetCurveSampler(const std::string CurveName)
{
	FCurveSampler CurveSampler;
	if (mCurveViewMap->count(CurveName))
	{
		CurveSampler.mCurveView = &mCurveViewMap->at(CurveName);
	}
	return CurveSampler;
}

FObjectAnimSampler::FObjectAnimSampler(const std::unordered_map<std::string, FCurveView>* InCurveViewMap):
	mCurveViewMap(InCurveViewMap)
{

	mLocationX = GetCurveSampler("LocationX");
	mLocationY = GetCurveSampler("LocationY");
	mLocationZ = GetCurveSampler("LocationZ");
	mRotationX = GetCurveSampler("RotationX");
	mRotationY = GetCurveSampler("RotationY");
	mRotationZ = GetCurveSampler("RotationZ");
	mScaleX = GetCurveSampler("ScaleX");
	mScaleY = GetCurveSampler("ScaleY");
	mScaleZ = GetCurveSampler("ScaleZ");
}

FTransform FObjectAnimSampler::SampleTransform(float TargetFrame) 
{
	FTransform Transform;

	Transform.Translation.x = mLocationX.SampleAnimDataByFrame(TargetFrame);
	Transform.Translation.y = mLocationY.SampleAnimDataByFrame(TargetFrame);
	Transform.Translation.z = mLocationZ.SampleAnimDataByFrame(TargetFrame);

	Transform.Rotation.x = mRotationX.SampleAnimDataByFrame(TargetFrame);
	Transform.Rotation.y = mRotationY.SampleAnimDataByFrame(TargetFrame);
	Transform.Rotation.z = mRotationZ.SampleAnimDataByFrame(TargetFrame);

	if (mScaleY.IsValid())
	{
		Transform.Scale.x = mScaleX.SampleAnimDataByFrame(TargetFrame);
	}
	if (mScaleX.IsValid())
	{
		Transform.Scale.y = mScaleY.SampleAnimDataByFrame(TargetFrame);
	}
	if (mScaleZ.IsValid())
	{
		Transform.Scale.z = mScaleZ.SampleAnimDataByFrame(TargetFrame);
	}

	return Transform;
}

float FObjectAnimSampler::SampleLocationX(float TargetFrame)
{
	return mLocationX.SampleAnimDataByFrame(TargetFrame);
}

float FObjectAnimSampler::SampleLocationY(float TargetFrame)
{
	return mLocationY.SampleAnimDataByFrame(TargetFrame);
}

float FObjectAnimSampler::SampleLocationZ(float TargetFrame)
{
	return mLocationZ.SampleAnimDataByFrame(TargetFrame);
}

XMFLOAT3 FObjectAnimSampler::SampleLocation(float TargetFrame)
{
	return XMFLOAT3(
		SampleLocationX(TargetFrame),
		SampleLocationY(TargetFrame),
		SampleLocationZ(TargetFrame)
	);
}


void WObjectAnimComponent::BeginComponent()
{
	Super::BeginComponent();
}

bool WObjectAnimComponent::LoadKeyframesFromOADAsset(const std::wstring& AssetName)
{
	if (FObjectAnimDataAsset* ObjectAnimData = FAssetManager::GetAsset<FObjectAnimDataAsset>(AssetName))
	{
		mFps = ObjectAnimData->GetFPS();
		mFrameEnd = ObjectAnimData->GetFraneEnd();

		const auto& ObjectCurveMap = ObjectAnimData->GetObjectCurveMap();
		for (const auto& Data : ObjectCurveMap)
		{
			mObjectAnimSamplerMap.insert({ Data.first, FObjectAnimSampler(&Data.second) });
		}

		return true;
	}
	
	return false;
}

FObjectAnimSampler* WObjectAnimComponent::GetObjectAnimSampler(std::string ObjectName)
{
	if (mObjectAnimSamplerMap.count(ObjectName))
	{
		return &mObjectAnimSamplerMap.at(ObjectName);
	}
	return nullptr;
}

void WObjectAnimComponent::GetObjectAnimSamplerList(TArray<std::string>& List)
{
	List.resize(mObjectAnimSamplerMap.size());
	int i = 0;
	for (const auto& Pair : mObjectAnimSamplerMap)
	{
		List[i++] = Pair.first;
	}
}

FTransform WObjectAnimComponent::SampleAnimWorldTransformByFrame(FObjectAnimSampler* Sampler, float Frame)
{
	XMMATRIX CM = GetWorldMatrix();
	XMMATRIX M = Sampler->SampleTransform(Frame).GetTransformMatrix();

	FTransform Transform;
	Transform.SetByTransformMatrix(M * CM);

	return Transform;
}

XMFLOAT3 WObjectAnimComponent::SampleAnimWorldLocationByFrame(FObjectAnimSampler* Sampler, float Frame)
{
	XMFLOAT3 Loc = Sampler->SampleLocation(Frame);
	XMStoreFloat3(&Loc, XMVector3Transform(XMLoadFloat3(&Loc), GetWorldMatrix()));
	return Loc;
}

FTransform WObjectAnimComponent::SampleAnimWorldTransformBySecond(FObjectAnimSampler* Sampler, float Second)
{
	return SampleAnimWorldTransformByFrame(Sampler, SecondToFrame(Second));
}

XMFLOAT3 WObjectAnimComponent::SampleAnimWorldLocationBySecond(FObjectAnimSampler* Sampler, float Second)
{
	return SampleAnimWorldLocationByFrame(Sampler, SecondToFrame(Second));
}
