#include "ObjectAnimComponent.h"
#include "Asset/AssetManager.h"
#include "Asset/XMLDataAsset.h"

using namespace tinyxml2;

void WObjectAnimComponent::LoadXML(const std::wstring& Name)
{
	if (FXMLDataAsset* Asset = dynamic_cast<FXMLDataAsset*>(FAssetManager::GetInstance()->GetAsset(Name)))
	{
		Doc = &Asset->Document;
	}

	if (Doc)
	{
		XMLElement* ProjectileAnimation = Doc->FirstChildElement();
		auto a = ProjectileAnimation->Value();
		if (!ProjectileAnimation)
		{
			return;
		}

		XMLElement* Info = ProjectileAnimation->FirstChildElement();
		mFPS = Info->FloatAttribute("fps");
		XMLElement* AnimationCurves = Info->NextSiblingElement();

		for (XMLElement* Curve = AnimationCurves->FirstChildElement(); Curve; Curve = Curve->NextSiblingElement())
		{
			const char* CurveName = Curve->Attribute("name");
			for (XMLElement* KeyframeElement = Curve->FirstChildElement(); KeyframeElement; KeyframeElement = KeyframeElement->NextSiblingElement())
			{
				FKeyframe Keyframe;
				Keyframe.Frame = KeyframeElement->FloatAttribute("frame");
				Keyframe.Value = KeyframeElement->FloatAttribute("value");
				Keyframe.LeftHandle.x = KeyframeElement->FloatAttribute("h_left_x");
				Keyframe.LeftHandle.y = KeyframeElement->FloatAttribute("h_left_y");
				Keyframe.RightHandle.x = KeyframeElement->FloatAttribute("h_right_x");
				Keyframe.RightHandle.y = KeyframeElement->FloatAttribute("h_right_y");

				const std::string InterpType = KeyframeElement->Attribute("interp");
				if (InterpType == "LINEAR")
				{
					Keyframe.Interpolation = EInterpolationType::EIT_Linear;
				}
				else if (InterpType == "BEZIER")
				{
					Keyframe.Interpolation = EInterpolationType::EIT_Bezier;
				}
				else if (InterpType == "CONSTANT")
				{
					Keyframe.Interpolation = EInterpolationType::EIT_Constant;
				}
				else
				{
					Keyframe.Interpolation = EInterpolationType::EIT_Undefined;
				}

				mLastFrame = max(mLastFrame, Keyframe.Frame);

				mKeyframeMap[CurveName].push_back(Keyframe);
			}
		}
	}
}

void WObjectAnimComponent::ToControlPoint(const FKeyframe& Left, const FKeyframe& Right, XMVECTOR* P0, XMVECTOR* P1, XMVECTOR* P2, XMVECTOR* P3) const
{
	XMFLOAT2 Point0 = { Left.Frame, Left.Value };
	const XMFLOAT2& Point1 = Left.RightHandle;
	const XMFLOAT2& Point2 = Right.LeftHandle;
	XMFLOAT2 Point3 = { Right.Frame, Right.Value };
	*P0 = XMLoadFloat2(&Point0);
	*P1 = XMLoadFloat2(&Point1);
	*P2 = XMLoadFloat2(&Point2);
	*P3 = XMLoadFloat2(&Point3);
}

float WObjectAnimComponent::InterpolateKeyframeByFrame(const std::vector<FKeyframe>& Keyframes, float TargetFrame) const
{
	size_t i = std::lower_bound(Keyframes.begin(), Keyframes.end(), TargetFrame,
		[](const FKeyframe& Keyframe, float Frame) { return Keyframe.Frame < Frame; }
	) - Keyframes.begin();

	if (i >= Keyframes.size())
	{
		return Keyframes.back().Value;
	}

	if (i <= 0)
	{
		return Keyframes[0].Value;
	}

	const FKeyframe& Left = Keyframes[i - 1];
	const FKeyframe& Right = Keyframes[i];
	assert(TargetFrame >= Left.Frame);
	assert(TargetFrame <= Right.Frame);
	assert(Left.Frame < Right.Frame);
	float Alpha = (TargetFrame - Left.Frame) / (Right.Frame - Left.Frame);

	float Value;
	switch (Left.Interpolation)
	{
	case EInterpolationType::EIT_Linear:
		Value = FDXMath::Lerp(Left.Value, Right.Value, Alpha);
		break;
	case EInterpolationType::EIT_Bezier:
	{
		XMVECTOR P0;
		XMVECTOR P1;
		XMVECTOR P2;
		XMVECTOR P3;
		ToControlPoint(Left, Right, &P0, &P1, &P2, &P3);
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

float WObjectAnimComponent::InterpolateKeyframeBySecond(const std::vector<FKeyframe>& Keyframes, float Second) const
{
	return InterpolateKeyframeByFrame(Keyframes, SecondToFrame(Second));
}

FTransform WObjectAnimComponent::GetKeyframeLocalTransformByFrame(float Frame) const
{
	FTransform Transform;

	// 1. Location (사용자 정의 이름: LocationX, LocationY, LocationZ)
	if (mKeyframeMap.count("LocationX")) Transform.Translation.x = InterpolateKeyframeByFrame(mKeyframeMap.at("LocationX"), Frame);
	if (mKeyframeMap.count("LocationY")) Transform.Translation.y = InterpolateKeyframeByFrame(mKeyframeMap.at("LocationY"), Frame);
	if (mKeyframeMap.count("LocationZ")) Transform.Translation.z = InterpolateKeyframeByFrame(mKeyframeMap.at("LocationZ"), Frame);

	// 2. Rotation (사용자 정의 이름: RotationX, RotationY, RotationZ, RotationW)
	if (mKeyframeMap.count("RotationX")) Transform.Rotation.x = InterpolateKeyframeByFrame(mKeyframeMap.at("RotationX"), Frame);
	if (mKeyframeMap.count("RotationY")) Transform.Rotation.y = InterpolateKeyframeByFrame(mKeyframeMap.at("RotationY"), Frame);
	if (mKeyframeMap.count("RotationZ")) Transform.Rotation.z = InterpolateKeyframeByFrame(mKeyframeMap.at("RotationZ"), Frame);

	// 3. Scale (사용자 정의 이름: ScaleX, ScaleY, ScaleZ)
	Transform.Scale = { 1.0f, 1.0f, 1.0f }; // 기본값 설정
	if (mKeyframeMap.count("ScaleX")) Transform.Scale.x = InterpolateKeyframeByFrame(mKeyframeMap.at("ScaleX"), Frame);
	if (mKeyframeMap.count("ScaleY")) Transform.Scale.y = InterpolateKeyframeByFrame(mKeyframeMap.at("ScaleY"), Frame);
	if (mKeyframeMap.count("ScaleZ")) Transform.Scale.z = InterpolateKeyframeByFrame(mKeyframeMap.at("ScaleZ"), Frame);

	return Transform;
}

FTransform WObjectAnimComponent::GetKeyframeLocalTransformBySecond(float Second) const
{
	return GetKeyframeLocalTransformByFrame(SecondToFrame(Second));
}

FTransform WObjectAnimComponent::GetKeyframeWorldTransformByFrame(float Frame)
{
	XMMATRIX CM = GetWorldMatrix();
	XMMATRIX M = GetKeyframeLocalTransformByFrame(Frame).GetTransformMatrix();

	FTransform Transform;
	Transform.SetByTransformMatrix(M * CM);

	return Transform;
}

FTransform WObjectAnimComponent::GetKeyframeWorldTransformBySecond(float Second)
{
	return GetKeyframeWorldTransformByFrame(SecondToFrame(Second));
}

float WObjectAnimComponent::GetPropertyByFrame(const std::string& PropertyName, float Frame) const
{
	if (mKeyframeMap.count(PropertyName))
	{
		return InterpolateKeyframeByFrame(mKeyframeMap.at(PropertyName), Frame);
	}
	return 0.0f;
}

float WObjectAnimComponent::GetPropertyBySecond(const std::string& PropertyName, float Second) const
{
	return GetPropertyByFrame(PropertyName, SecondToFrame(Second));
}
