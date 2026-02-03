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

	RegisterWFunction("LoadAnimation", [this](const WFunctionParams& Params)
		{
			std::string AssetName;
			std::string AnimName;
			for (const auto Param : Params)
			{
				if (Param->Name == "AssetName")
				{
					AssetName = Param->Get<std::string>();
				}
				else if (Param->Name == "AnimName")
				{
					AnimName = Param->Get<std::string>();
				}
			}
			LoadAnimation(AssetName, AnimName);
			return nullptr;
		}
	);

	BEGIN_WFUNCTION(SetTargetComponent)
	{
		WSceneComponent* Component = nullptr;
		for (const auto& Param : Params)
		{
			if (Param->Name == "Component")
			{
				Component = Param->Get<WSceneComponent*>();
			}
		}
		SetTargetComponent(Component);
		return nullptr;
	}
	END_WFUNCTION

	RegisterWFunction("Play", [this](const WFunctionParams& Params)
		{
			bool bLoop = false;
			uint16_t Flags = 0;

			for (const auto& Param : Params)
			{
				if (Param->Name == "Loop")
				{
					bLoop = Param->Get<bool>();
				}
				else if (Param->Name == "Loc")
				{
					std::string LocFlag = Param->Get<std::string>();
					if (LocFlag.find('X') != std::string::npos)
					{
						Flags |= ERootMotion::LocX;
					}
					if (LocFlag.find('Y') != std::string::npos)
					{
						Flags |= ERootMotion::LocY;
					}
					if (LocFlag.find('Z') != std::string::npos)
					{
						Flags |= ERootMotion::LocZ;
					}
				}
				else if (Param->Name == "Rot")
				{
					std::string RotFlag = Param->Get<std::string>();
					if (RotFlag.find('X') != std::string::npos)
					{
						Flags |= ERootMotion::RotX;
					}
					if (RotFlag.find('Y') != std::string::npos)
					{
						Flags |= ERootMotion::RotY;
					}
					if (RotFlag.find('Z') != std::string::npos)
					{
						Flags |= ERootMotion::RotZ;
					}
				}
				else if (Param->Name == "Scale")
				{
					std::string ScaleFlag = Param->Get<std::string>();
					if (ScaleFlag.find('X') != std::string::npos)
					{
						Flags |= ERootMotion::ScaleX;
					}
					if (ScaleFlag.find('Y') != std::string::npos)
					{
						Flags |= ERootMotion::ScaleY;
					}
					if (ScaleFlag.find('Z') != std::string::npos)
					{
						Flags |= ERootMotion::ScaleZ;
					}
				}
			}

			Play(bLoop, Flags == 0 ? ERootMotion::All : Flags);
			return nullptr;
		}
	);
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
	mCurrentTime += DeltaTime;
	float duration = GetDuration();

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
			mCurrentTime = std::fmod(mCurrentTime, duration);
			FTransform startT = mSampler->SampleTransform(0.0f);
			FTransform currT = mSampler->SampleTransform(SecondToFrame(mCurrentTime));

			vDeltaLoc = (XMLoadFloat3(&endT.Translation) - XMLoadFloat3(&prevT.Translation)) +
				(XMLoadFloat3(&currT.Translation) - XMLoadFloat3(&startT.Translation));

			vDeltaRot = (XMLoadFloat3(&endT.Rotation) - XMLoadFloat3(&prevT.Rotation)) +
				(XMLoadFloat3(&currT.Rotation) - XMLoadFloat3(&startT.Rotation));

			vDeltaScale = (XMLoadFloat3(&endT.Scale) - XMLoadFloat3(&prevT.Scale)) +
				(XMLoadFloat3(&currT.Scale) - XMLoadFloat3(&startT.Scale));
		}
		else
		{
			// 종료 처리 (기존 로직 유지)
			mCurrentTime = duration;
			mIsPlaying = false;

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
		FTransform currT = mSampler->SampleTransform(SecondToFrame(mCurrentTime));

		vDeltaLoc = XMLoadFloat3(&currT.Translation) - XMLoadFloat3(&prevT.Translation);
		vDeltaRot = XMLoadFloat3(&currT.Rotation) - XMLoadFloat3(&prevT.Rotation);
		vDeltaScale = XMLoadFloat3(&currT.Scale) - XMLoadFloat3(&prevT.Scale);
	}

	// 비트 플래그에 따른 마스킹
	XMFLOAT3 dLoc, dRot, dScale;
	XMStoreFloat3(&dLoc, vDeltaLoc);
	XMStoreFloat3(&dRot, vDeltaRot);
	XMStoreFloat3(&dScale, vDeltaScale);

	if (!(mRootMotionFlags & ERootMotion::LocX)) dLoc.x = 0.f;
	if (!(mRootMotionFlags & ERootMotion::LocY)) dLoc.y = 0.f;
	if (!(mRootMotionFlags & ERootMotion::LocZ)) dLoc.z = 0.f;

	if (!(mRootMotionFlags & ERootMotion::RotX)) dRot.x = 0.f;
	if (!(mRootMotionFlags & ERootMotion::RotY)) dRot.y = 0.f;
	if (!(mRootMotionFlags & ERootMotion::RotZ)) dRot.z = 0.f;

	if (!(mRootMotionFlags & ERootMotion::ScaleX)) dScale.x = 0.f;
	if (!(mRootMotionFlags & ERootMotion::ScaleY)) dScale.y = 0.f;
	if (!(mRootMotionFlags & ERootMotion::ScaleZ)) dScale.z = 0.f;

	// 4. 최종 적용 대상 결정 (캐싱된 타겟이 없으면 액터)
	if (auto Target = mTarget.lock()) // 특정 씬 컴포넌트를 조종할 때
	{
		// 컴포넌트는 상대(Local) 좌표계이므로 델타를 그냥 더해줌
		FTransform T = Target->GetLocalTransform();

		XMVECTOR NewLoc = XMLoadFloat3(&T.Translation) + XMLoadFloat3(&dLoc);
		XMVECTOR NewRot = XMLoadFloat3(&T.Rotation) + XMLoadFloat3(&dRot);
		XMVECTOR NewScale = XMLoadFloat3(&T.Scale) + XMLoadFloat3(&dScale);

		FTransform NewT;
		XMStoreFloat3(&NewT.Translation, NewLoc);
		XMStoreFloat3(&NewT.Rotation, NewRot);
		XMStoreFloat3(&NewT.Scale, NewScale);

		Target->SetLocalTransform(NewT);
	}
	else if(AActor* OwnerPtr = GetOwner().lock().get()) // 타겟이 없으면 액터(RootComponent)에 적용 (루트 모션)
	{
		// 위치는 액터의 현재 회전 방향을 고려해서 더해줘야 함
		XMFLOAT4 ActorQuat = OwnerPtr->GetActorQuaternion();
		XMVECTOR vActorQuat = XMLoadFloat4(&ActorQuat);
		XMVECTOR vRotatedDeltaLoc = XMVector3Rotate(XMLoadFloat3(&dLoc), vActorQuat);

		// 위치 적용
		XMFLOAT3 curLoc = OwnerPtr->GetActorLocation();
		XMVECTOR vNewLoc = XMLoadFloat3(&curLoc) + vRotatedDeltaLoc;
		XMFLOAT3 finalLoc;
		XMStoreFloat3(&finalLoc, vNewLoc);
		OwnerPtr->SetActorLocation(finalLoc);

		// 회전 적용 (Euler 누적)
		XMFLOAT3 curRot = OwnerPtr->GetActorRotation();
		XMVECTOR vNewRot = XMLoadFloat3(&curRot) + XMLoadFloat3(&dRot);
		XMFLOAT3 finalRot;
		XMStoreFloat3(&finalRot, vNewRot);
		OwnerPtr->SetActorRotation(finalRot);

		// 스케일 적용
		XMFLOAT3 curScale = OwnerPtr->GetActorScale();
		XMVECTOR vNewScale = XMLoadFloat3(&curScale) + XMLoadFloat3(&dScale);
		XMFLOAT3 finalScale;
		XMStoreFloat3(&finalScale, vNewScale);
		OwnerPtr->SetActorScale(finalScale);
	}
}

bool WObjectAnimComponent::LoadAnimation(const std::string& AssetName, const std::string& AnimName)
{
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

void WObjectAnimComponent::Play(bool bLoop, uint16_t Flags)
{
	mIsPlaying = true;
	mLoop = bLoop;
	mRootMotionFlags = Flags;
}

void WObjectAnimComponent::Stop()
{
	mIsPlaying = false;
	mCurrentTime = 0.0f;
}

void WObjectAnimComponent::SetTargetComponent(WSceneComponent* Comp)
{
	if (Comp)
	{
		mTarget = Comp->GetWeakPtr<WSceneComponent>();
	}
}
