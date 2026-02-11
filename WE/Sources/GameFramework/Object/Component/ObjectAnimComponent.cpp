#include "ObjectAnimComponent.h"
#include "Asset/AssetManager.h"
#include "Asset/XMLDataAsset.h"
#include "Asset/AssetLoader.h"
#include "Actor/Actor.h"

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

FObjectAnimSampler::FObjectAnimSampler(const std::unordered_map<std::string, FCurveView>* InCurveViewMap) :
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

FCurveSampler FObjectAnimSampler::GetCurveSampler(const std::string CurveName)
{
	FCurveSampler CurveSampler;
	if (mCurveViewMap->count(CurveName))
	{
		CurveSampler.mCurveView = &mCurveViewMap->at(CurveName);
	}
	return CurveSampler;
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

WObjectAnimComponent::WObjectAnimComponent()
{
	SetTickGroup(ETickGroup::ETG_PrePhysics, ETickPriority::ETP_Low);
}

void WObjectAnimComponent::BeginComponent()
{
	Super::BeginComponent();
}

void WObjectAnimComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!mIsPlaying || !mSampler)
	{
		return;
	}

	// 1. 시간 업데이트 (이전 프레임 기록)
	float prevTime = mCurrentTime;
	mCurrentTime += DeltaTime * mPlayRate;
	float FinalTime = mCurrentTime;
	float duration = GetDuration();

	if (mCurrentTime >= duration)
	{
		if (mLoop)
		{
			FinalTime = std::fmod(mCurrentTime, duration);
		}
		else
		{
			Stop();
		}
	}	

	if (mSamplingFlags > 0)
	{
		if (mbRootMotion)
		{
			XMVECTOR vDeltaLoc;
			XMVECTOR vDeltaRot;
			XMVECTOR vDeltaScale;

			if (mCurrentTime >= duration)
			{
				// 루프 시점 델타 보정
				if (mLoop)
				{
					// A 구간: 이전 시간부터 끝까지의 움직임
					FTransform prevT = mSampler->SampleTransform(SecondToFrame(prevTime));
					FTransform endT = mSampler->SampleTransform(SecondToFrame(duration));

					// B 구간: 처음부터 현재 리셋된 시간까지의 움직임
					FTransform startT = mSampler->SampleTransform(0.0f);
					FTransform currT = mSampler->SampleTransform(SecondToFrame(FinalTime));

					vDeltaLoc = (XMLoadFloat3(&endT.Translation) - XMLoadFloat3(&prevT.Translation)) +
						(XMLoadFloat3(&currT.Translation) - XMLoadFloat3(&startT.Translation));

					vDeltaRot = (XMLoadFloat3(&endT.Rotation) - XMLoadFloat3(&prevT.Rotation)) +
						(XMLoadFloat3(&currT.Rotation) - XMLoadFloat3(&startT.Rotation));

					vDeltaScale = (XMLoadFloat3(&endT.Scale) - XMLoadFloat3(&prevT.Scale)) +
						(XMLoadFloat3(&currT.Scale) - XMLoadFloat3(&startT.Scale));
				}
				else
				{
					FTransform prevT = mSampler->SampleTransform(SecondToFrame(prevTime));
					FTransform endT = mSampler->SampleTransform(SecondToFrame(duration));
					vDeltaLoc = XMLoadFloat3(&endT.Translation) - XMLoadFloat3(&prevT.Translation);
					vDeltaRot = XMLoadFloat3(&endT.Rotation) - XMLoadFloat3(&prevT.Rotation);
					vDeltaScale = XMLoadFloat3(&endT.Scale) - XMLoadFloat3(&prevT.Scale);
				}
			}
			else
			{
				// 일반 프레임 델타 계산 (기존 로직)
				FTransform prevT = mSampler->SampleTransform(SecondToFrame(prevTime));
				FTransform currT = mSampler->SampleTransform(SecondToFrame(FinalTime));

				vDeltaLoc = XMLoadFloat3(&currT.Translation) - XMLoadFloat3(&prevT.Translation);
				vDeltaRot = XMLoadFloat3(&currT.Rotation) - XMLoadFloat3(&prevT.Rotation);
				vDeltaScale = XMLoadFloat3(&currT.Scale) - XMLoadFloat3(&prevT.Scale);
			}

			// 비트 플래그에 따른 마스킹
			XMFLOAT3 dLoc, dRot, dScale;
			XMStoreFloat3(&dLoc, vDeltaLoc);
			XMStoreFloat3(&dRot, vDeltaRot);
			XMStoreFloat3(&dScale, vDeltaScale);

			if (!(mSamplingFlags & EAnimSampling::LocX)) dLoc.x = 0.f;
			if (!(mSamplingFlags & EAnimSampling::LocY)) dLoc.y = 0.f;
			if (!(mSamplingFlags & EAnimSampling::LocZ)) dLoc.z = 0.f;

			if (!(mSamplingFlags & EAnimSampling::RotX)) dRot.x = 0.f;
			if (!(mSamplingFlags & EAnimSampling::RotY)) dRot.y = 0.f;
			if (!(mSamplingFlags & EAnimSampling::RotZ)) dRot.z = 0.f;

			if (!(mSamplingFlags & EAnimSampling::ScaleX)) dScale.x = 0.f;
			if (!(mSamplingFlags & EAnimSampling::ScaleY)) dScale.y = 0.f;
			if (!(mSamplingFlags & EAnimSampling::ScaleZ)) dScale.z = 0.f;


			XMFLOAT4 CurrQuat = GetLocalQuatRotation();
			XMVECTOR vCurrQuat = XMLoadFloat4(&CurrQuat);
			XMVECTOR vRotatedDeltaLoc = XMVector3Rotate(XMLoadFloat3(&dLoc), vCurrQuat);

			// 위치 적용
			XMFLOAT3 curLoc = GetLocalLocation();
			XMVECTOR vNewLoc = XMLoadFloat3(&curLoc) + vRotatedDeltaLoc;
			XMFLOAT3 finalLoc;
			XMStoreFloat3(&finalLoc, vNewLoc);
			SetLocalLocation(finalLoc);

			// 회전 적용 (Euler 누적)
			XMFLOAT3 curRot = GetLocalRotation();
			XMVECTOR vNewRot = XMLoadFloat3(&curRot) + XMLoadFloat3(&dRot);
			XMFLOAT3 finalRot;
			XMStoreFloat3(&finalRot, vNewRot);
			SetLocalRotation(finalRot);

			// 스케일 적용
			XMFLOAT3 curScale = GetLocalScale();
			XMVECTOR vNewScale = XMLoadFloat3(&curScale) + XMLoadFloat3(&dScale);
			XMFLOAT3 finalScale;
			XMStoreFloat3(&finalScale, vNewScale);
			SetLocalScale(finalScale);
		}

		else
		{
			// 1. 현재 시간에 해당하는 애니메이션 데이터 샘플링 (절대 좌표)
			FTransform currT = mSampler->SampleTransform(SecondToFrame(FinalTime));

			// 2. 비트 플래그(mSamplingFlags)에 따라 사용할 값 필터링
			// 팁: 루트모션이 아닐 때는 이전 값과 섞지 않고 샘플링된 값을 그대로 사용하거나, 
			// 플래그가 꺼진 축은 현재 컴포넌트의 값을 유지하도록 설계하는 것이 일반적입니다.

			XMFLOAT3 finalLoc = GetLocalLocation();
			XMFLOAT3 finalRot = GetLocalRotation();
			XMFLOAT3 finalScale = GetLocalScale();

			// 위치 (Location) 샘플링 적용
			if (mSamplingFlags & EAnimSampling::LocX) finalLoc.x = currT.Translation.x;
			if (mSamplingFlags & EAnimSampling::LocY) finalLoc.y = currT.Translation.y;
			if (mSamplingFlags & EAnimSampling::LocZ) finalLoc.z = currT.Translation.z;

			// 회전 (Rotation - Euler) 샘플링 적용
			if (mSamplingFlags & EAnimSampling::RotX) finalRot.x = currT.Rotation.x;
			if (mSamplingFlags & EAnimSampling::RotY) finalRot.y = currT.Rotation.y;
			if (mSamplingFlags & EAnimSampling::RotZ) finalRot.z = currT.Rotation.z;

			// 스케일 (Scale) 샘플링 적용
			if (mSamplingFlags & EAnimSampling::ScaleX) finalScale.x = currT.Scale.x;
			if (mSamplingFlags & EAnimSampling::ScaleY) finalScale.y = currT.Scale.y;
			if (mSamplingFlags & EAnimSampling::ScaleZ) finalScale.z = currT.Scale.z;

			// 3. 최종 트랜스폼 적용 (절대값 덮어쓰기)
			SetLocalLocation(finalLoc);
			SetLocalRotation(finalRot);
			SetLocalScale(finalScale);
		}
	}


	for (FCurveBind& BoundCurve : mBoundCurves)
	{
		float Value = BoundCurve.CurveSampler.SampleAnimDataByFrame(SecondToFrame(FinalTime));
		if (BoundCurve.bModifier)
		{
			Value *= BoundCurve.BaseValue;
		}

		*BoundCurve.TargetPtr = Value;
	}


	// 항상 마지막에 오도록
	mCurrentTime = FinalTime;
}

bool WObjectAnimComponent::LoadAnimation(const std::string& AssetName, const std::string& AnimName)
{
	mBoundCurves.clear();
	if (FObjectAnimDataAsset* ObjectAnimData = FAssetManager::GetAsset<FObjectAnimDataAsset>(AssetName))
	{
		mFps = ObjectAnimData->GetFPS();
		mFrameEnd = ObjectAnimData->GetFraneEnd();

		const auto& ObjectCurveMap = ObjectAnimData->GetObjectCurveMap();
		mSampler = MakeUnique<FObjectAnimSampler>(&ObjectCurveMap.at(AnimName));
		
		return mSampler != nullptr;
	}

	return false;
}

void WObjectAnimComponent::LoadAndPlay(const std::string& AssetName, const std::string& AnimName, float PlayRate, bool bLoop, uint16_t Flags, bool bRootMotion)
{
	LoadAnimation(AssetName, AnimName);
	Play(PlayRate, bLoop, Flags, bRootMotion);
}

void WObjectAnimComponent::BindCurve(const std::string& CurveName, float* TargetPtr, bool bModifier, float BaseValue)
{
	if (TargetPtr == nullptr)
	{
		return;
	}

	FCurveBind* BoundCurve;
	int i = GetBoundCurveIndex(TargetPtr);
	if (i < mBoundCurves.size())
	{
		BoundCurve = &mBoundCurves[i];
	}
	else
	{
		i = (int)mBoundCurves.size();
		mBoundCurves.push_back({});
		BoundCurve = &mBoundCurves.back();
		BoundCurve->TargetPtr = TargetPtr;
	}

	FCurveSampler CurveSampler = mSampler->GetCurveSampler(CurveName);
	if (!CurveSampler.IsValid())
	{
		RemoveBoundCurveAt(i);
		return;
	}

	BoundCurve->CurveSampler = CurveSampler;
	BoundCurve->bModifier = bModifier;
	BoundCurve->BaseValue = BaseValue;
}

void WObjectAnimComponent::RemoveBoundCurveAt(int i)
{
	if (i < mBoundCurves.size() - 1)
	{
		mBoundCurves[i] = std::move(mBoundCurves.back());
	}

	mBoundCurves.pop_back();
}

int WObjectAnimComponent::GetBoundCurveIndex(float* TargetPtr)
{
	return (int)(std::find_if(mBoundCurves.begin(), mBoundCurves.end(), [=](const FCurveBind& BoundCurve)
		{
			return BoundCurve.TargetPtr == TargetPtr;
		}
	) - mBoundCurves.begin());
}

WObjectAnimComponent::FCurveBind* WObjectAnimComponent::GetBoundCurve(float* TargetPtr)
{
	int i = GetBoundCurveIndex(TargetPtr);
	return i < mBoundCurves.size() ? &mBoundCurves[i] : nullptr;
}

void WObjectAnimComponent::Play(float PlayRate, bool bLoop, uint16_t Flags, bool bRootMotion)
{
	mIsPlaying = true;
	mLoop = bLoop;
	mSamplingFlags = Flags;
	mPlayRate = max(0.001f, PlayRate);
	mbRootMotion = bRootMotion;
}

void WObjectAnimComponent::Stop()
{
	mIsPlaying = false;
	mCurrentTime = 0.0f;

	mOnStop.Broadcast();
}
