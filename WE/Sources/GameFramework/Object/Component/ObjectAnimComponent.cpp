#include "ObjectAnimComponent.h"
#include "Asset/AssetManager.h"
#include "Asset/XMLDataAsset.h"
#include "Asset/AssetLoader.h"


using namespace tinyxml2;

bool WObjectAnimComponent::LoadKeyframesFromXMLAsset(const std::wstring& Name)
{
	mKeyframeMap.clear();
	tinyxml2::XMLDocument* Doc = nullptr;
	if (FXMLDataAsset* Asset = dynamic_cast<FXMLDataAsset*>(FAssetManager::GetInstance()->GetAsset(Name)))
	{
		Doc = &Asset->Document;
	}

	return LoadKeyframesFromXML(Doc);
}

bool WObjectAnimComponent::LoadKeyframesFromXML(tinyxml2::XMLDocument* Doc)
{
	mKeyframeMap.clear();
	std::string ElementNameSet[2][7] = {
		{"frame", "value", "interp", "h_left_x", "h_left_y", "h_right_x", "h_right_y"},			// v1
		{"f", "v", "i", "hlx", "hly", "hrx", "hry"}
	};
	if (Doc)
	{
		XMLElement* ObjectAnimation = Doc->FirstChildElement();
		int ElementNameSetIndex = 0;
		if (const char* s = ObjectAnimation->Attribute("version"))
		{
			std::string Version = ObjectAnimation->Attribute("version");
			
			if (Version == "v1")
			{
				ElementNameSetIndex = 0;
			}
			else if (Version == "v2")
			{
				ElementNameSetIndex = 1;
			}
		}

		if (!ObjectAnimation)
		{
			return false;
		}

		XMLElement* Info = ObjectAnimation->FirstChildElement();
		mFPS = Info->FloatAttribute("fps");
		float FrameStart = Info->FloatAttribute("frame_start");
		mFrameEnd = Info->FloatAttribute("frame_end") - FrameStart;
		XMLElement* AnimationCurves = Info->NextSiblingElement();

		for (XMLElement* Curve = AnimationCurves->FirstChildElement(); Curve; Curve = Curve->NextSiblingElement())
		{
			const char* CurveName = Curve->Attribute("name");
			for (XMLElement* KeyframeElement = Curve->FirstChildElement(); KeyframeElement; KeyframeElement = KeyframeElement->NextSiblingElement())
			{
				FKeyframe Keyframe;
				Keyframe.Frame = KeyframeElement->FloatAttribute(ElementNameSet[ElementNameSetIndex][0].c_str()) - FrameStart;
				Keyframe.Value = KeyframeElement->FloatAttribute(ElementNameSet[ElementNameSetIndex][1].c_str());
				Keyframe.LeftHandle.x = KeyframeElement->FloatAttribute(ElementNameSet[ElementNameSetIndex][3].c_str()) - FrameStart;
				Keyframe.LeftHandle.y = KeyframeElement->FloatAttribute(ElementNameSet[ElementNameSetIndex][4].c_str());
				Keyframe.RightHandle.x = KeyframeElement->FloatAttribute(ElementNameSet[ElementNameSetIndex][5].c_str()) - FrameStart;
				Keyframe.RightHandle.y = KeyframeElement->FloatAttribute(ElementNameSet[ElementNameSetIndex][6].c_str());

				const std::string InterpType = KeyframeElement->Attribute(ElementNameSet[ElementNameSetIndex][2].c_str());
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

				mKeyframeMap[CurveName].push_back(Keyframe);
			}
		}

		return true;
	}
	return false;
}

bool WObjectAnimComponent::LoadKeyframesFromXMLFile(const std::string& FilePath)
{
	mKeyframeMap.clear();
	tinyxml2::XMLDocument Doc;
	if (Asset::LoadZLibXML(FilePath, Doc))
	{
		return LoadKeyframesFromXML(&Doc);
	}
	
	return false;	
}

bool WObjectAnimComponent::LoadKeyframesFromZlibXMLFile(const std::string& FilePath)
{
	mKeyframeMap.clear();
	tinyxml2::XMLDocument Doc;
	if (Asset::LoadZLibXML(FilePath, Doc))
	{
		return LoadKeyframesFromXML(&Doc);
	}
	return false;
}

bool WObjectAnimComponent::LoadKeyframesFromZlibKeyframeMap(const std::string& FilePath)
{
	std::vector<unsigned char> RawBuffer;
	// 1. zlib 해제 (헤더 4바이트 읽고 나머지를 해제하는 기존 함수)
	if (!Asset::LoadZlib(FilePath, RawBuffer))
	{
		return false;
	}

	return LoadKeyframesFromBinary(RawBuffer.data());
}

bool WObjectAnimComponent::LoadKeyframesFromLZ4KeyframeMap(const std::string& FilePath)
{
	std::vector<unsigned char> RawBuffer;
	// 1. zlib 해제 (헤더 4바이트 읽고 나머지를 해제하는 기존 함수)
	if (!Asset::LoadLZ4(FilePath, RawBuffer))
	{
		return false;
	}

	return LoadKeyframesFromBinary(RawBuffer.data());
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

bool WObjectAnimComponent::LoadKeyframesFromBinary(unsigned char* Ptr)
{
	if (Ptr == nullptr)
	{
		return false;
	}

	mKeyframeMap.clear();

	// 프레임 정보 읽기
	mFPS = *reinterpret_cast<float*>(Ptr);
	Ptr += sizeof(float);
	mFrameEnd = *reinterpret_cast<float*> (Ptr);
	Ptr += sizeof(float);

	// 2. 전체 커브 개수 읽기
	int TotalCurveNum = *reinterpret_cast<int*>(Ptr);
	Ptr += sizeof(int);

	for (int i = 0; i < TotalCurveNum; ++i)
	{
		// 3. 커브 이름 읽기
		int CurveNameLen = *reinterpret_cast<int*>(Ptr);
		Ptr += sizeof(int);

		std::string CurveName(reinterpret_cast<char*>(Ptr), CurveNameLen);
		Ptr += CurveNameLen;

		// 4. 키프레임 개수 읽기
		int TotalKeyframeNum = *reinterpret_cast<int*>(Ptr);
		Ptr += sizeof(int);

		// 5. 키프레임 데이터 통째로 로드
		mKeyframeMap[CurveName].resize(TotalKeyframeNum);
		size_t DataSize = sizeof(FKeyframe) * TotalKeyframeNum;

		// 메모리 통째로 복사 (이게 XML 파싱보다 압도적으로 빠릅니다)
		memcpy(mKeyframeMap[CurveName].data(), Ptr, DataSize);
		Ptr += DataSize;
	}

	return true;
}
