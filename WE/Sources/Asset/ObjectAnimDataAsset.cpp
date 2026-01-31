#include "ObjectAnimDataAsset.h"
#include <lz4.h>
#include <tinyxml2.h>
#include <filesystem>
#include "Utility/String.h"
#include "AssetLoader.h"
#include "GUI/GUICore.h"
#include "Utility/Timer.h"

namespace fs = std::filesystem;

bool CompileXMLToOADLz4(const std::wstring& XMLPath, const std::wstring& OADLz4Path)
{
	tinyxml2::XMLDocument Doc;
	if (!Asset::LoadXML(XMLPath, Doc))
	{
		return false;
	}

	tinyxml2::XMLElement* Root = Doc.FirstChildElement(); // AnimationAsset 등
	if (!Root)
	{
		return false;
	}

	tinyxml2::XMLElement* InfoNode = Root->FirstChildElement("Info");

	if (!InfoNode)
	{
		return false;
	}

	std::vector<char> Payload;
	auto Append = [&](const void* data, size_t size) {
		const char* p = static_cast<const char*>(data);
		Payload.insert(Payload.end(), p, p + size);
	};

	// 0. Info (FPS, Duration)
	float fps = InfoNode->FloatAttribute("fps");
	float fStart = InfoNode->FloatAttribute("frame_start");
	float fEnd = InfoNode->FloatAttribute("frame_end");
	float Duration = fEnd - fStart;
	Append(&fps, sizeof(float));
	Append(&Duration, sizeof(float));


	tinyxml2::XMLElement* ObjectElement = Root->FirstChildElement("Object");
	int ObjectCount = 0;
	for (auto* o = ObjectElement; o; o = o->NextSiblingElement("Object"))
	{
		++ObjectCount;
	}
	Append(&ObjectCount, sizeof(int));
	while (ObjectElement)
	{
		std::string ObjectName = ObjectElement->Attribute("name");
		int ObjectNameLen = (int)ObjectName.length();
		Append(&ObjectNameLen, sizeof(int));
		Append(ObjectName.c_str(), ObjectNameLen);

		tinyxml2::XMLElement* CurveElement = ObjectElement->FirstChildElement("Curve");

		// 1. Curve 개수
		int CurveCount = 0;
		for (auto* c = ObjectElement->FirstChildElement("Curve"); c; c = c->NextSiblingElement("Curve")) CurveCount++;
		Append(&CurveCount, sizeof(int));

		// 2. Curve 데이터 순회
		while(CurveElement)
		{
			std::string CurveName = CurveElement->Attribute("name");
			int CurveNameLen = (int)CurveName.length();
			Append(&CurveNameLen, sizeof(int));
			Append(CurveName.c_str(), CurveNameLen);

			int KeyframeCount = 0;
			for (auto* k = CurveElement->FirstChildElement("Keyframe"); k; k = k->NextSiblingElement("Keyframe")) KeyframeCount++;
			Append(&KeyframeCount, sizeof(int));

			int i = 0;
			std::vector<float> Frames(KeyframeCount);
			std::vector<FKeyframeData> Datas(KeyframeCount);
			for (auto* KeyframeElement = CurveElement->FirstChildElement("Keyframe"); KeyframeElement; KeyframeElement = KeyframeElement->NextSiblingElement("Keyframe"), ++i)
			{
				FKeyframeData& Data = Datas[i];
				float& Frame = Frames[i];
				Frame = KeyframeElement->FloatAttribute("frame") - fStart;
				Data.Value = KeyframeElement->FloatAttribute("value");

				const char* interp = KeyframeElement->Attribute("interp");
				if (strcmp(interp, "LINEAR") == 0) Data.Interpolation = EInterpolationType::EIT_Linear;
				else if (strcmp(interp, "BEZIER") == 0) Data.Interpolation = EInterpolationType::EIT_Bezier;
				else if (strcmp(interp, "CONSTANT") == 0) Data.Interpolation = EInterpolationType::EIT_Constant;
				else Data.Interpolation = EInterpolationType::EIT_Undefined;

				Data.RightHandle.x = KeyframeElement->FloatAttribute("h_right_x") - fStart;
				Data.RightHandle.y = KeyframeElement->FloatAttribute("h_right_y");
				Data.LeftHandle.x = KeyframeElement->FloatAttribute("h_left_x") - fStart;
				Data.LeftHandle.y = KeyframeElement->FloatAttribute("h_left_y");
			}
			Append(Frames.data(), sizeof(float) * KeyframeCount);
			Append(Datas.data(), sizeof(FKeyframeData) * KeyframeCount);

			CurveElement = CurveElement->NextSiblingElement("Curve");
		}

		ObjectElement = ObjectElement->NextSiblingElement("Object");
	}

	// 3. LZ4 압축
	int originalSize = (int)Payload.size();
	int maxBound = LZ4_compressBound(originalSize);
	std::vector<char> compressed(maxBound);
	int compressedSize = LZ4_compress_default(Payload.data(), compressed.data(), originalSize, maxBound);

	// 4. 저장 (Header: originalSize + Data: compressed)
	std::ofstream outFile(OADLz4Path, std::ios::binary);
	if (outFile.is_open())
	{
		outFile.write((char*)&originalSize, sizeof(int));
		outFile.write(compressed.data(), compressedSize);
		outFile.close();
		OutputDebugStringW((L"OAD Compile Success: " + OADLz4Path + L"\n").c_str());
	}

	return true;
}

bool FObjectAnimDataAsset::LoadAsset(const std::wstring& FilePath)
{
	// 경로 설정
	std::wstring xmlPath = FilePath;
	std::wstring lz4Path = xmlPath.substr(0, xmlPath.find_last_of('.')) + L".oad.lz4";

	bool bNeedCompile = false;

	// 파일 존재 여부 및 업데이트 시간 체크
	if (!fs::exists(lz4Path))
	{
		bNeedCompile = true;
	}
	else if (fs::exists(xmlPath))
	{
		auto xmlTime = fs::last_write_time(xmlPath);
		auto lz4Time = fs::last_write_time(lz4Path);

		if (xmlTime > lz4Time) // XML이 더 최신이면
			bNeedCompile = true;
	}

	// 필요 시 컴파일 수행
	if (bNeedCompile)
	{
		if (fs::exists(xmlPath))
		{
			CompileXMLToOADLz4(xmlPath, lz4Path);
		}
		else
		{
			// XML도 없고 LZ4도 없으면 로드 실패
			return false;
		}
	}

	// LZ4 파일로부터 데이터 로드
	if (!Asset::LoadLZ4(lz4Path, mRawBuffer))
	{
		return false;
	}

	unsigned char* Ptr = mRawBuffer.data();

	// 프레임 정보 읽기
	mFPS = *reinterpret_cast<float*>(Ptr);
	Ptr += sizeof(float);
	mDuration = *reinterpret_cast<float*> (Ptr);
	Ptr += sizeof(float);

	int TotalObjectNum = *reinterpret_cast<int*>(Ptr);
	Ptr += sizeof(int);

	for (int i = 0; i < TotalObjectNum; ++i)
	{
		int ObjectNameLen = *reinterpret_cast<int*>(Ptr);
		Ptr += sizeof(int);
		std::string ObjectName(reinterpret_cast<char*>(Ptr), ObjectNameLen);
		Ptr += ObjectNameLen;

		// 전체 커브 개수 읽기
		int TotalCurveNum = *reinterpret_cast<int*>(Ptr);
		Ptr += sizeof(int);

		mCurvesStartPtr = Ptr;

		for (int j = 0; j < TotalCurveNum; ++j)
		{
			FCurveView CurveInfo;
			CurveInfo.StartPtr = Ptr;

			// 커브 이름 읽기
			int CurveNameLen = *reinterpret_cast<int*>(Ptr);
			Ptr += sizeof(int);

			std::string CurveName(reinterpret_cast<char*>(Ptr), CurveNameLen);
			Ptr += CurveNameLen;

			// 키프레임 개수 읽기
			CurveInfo.TotalKeyFrameNum = *reinterpret_cast<int*>(Ptr);
			Ptr += sizeof(int);

			// 키프레임 데이터 시작점 포인터 저장
			size_t DataSize = sizeof(float) * CurveInfo.TotalKeyFrameNum;
			CurveInfo.FramesPtr = reinterpret_cast<float*>(Ptr);
			Ptr += DataSize;

			DataSize = sizeof(FKeyframeData) * CurveInfo.TotalKeyFrameNum;
			CurveInfo.KeyframeDatasPtr = reinterpret_cast<FKeyframeData*>(Ptr);
			Ptr += DataSize;

			mObjectCurveMap[ObjectName][CurveName] = CurveInfo;
		}
	}

	return true;
}

const FCurveView* FObjectAnimDataAsset::GetCurveInfoSafe(const std::string& ObjectName, const std::string& CurveName) const
{
	if (mObjectCurveMap.count(ObjectName))
	{
		if (mObjectCurveMap.at(ObjectName).count(CurveName))
		{
			return &mObjectCurveMap.at(ObjectName).at(CurveName);
		}
	}
	return nullptr;
}

void FObjectAnimDataAsset::GetObjectNames(std::vector<std::string>& Names)
{
	Names.clear();

	for (auto Pair : mObjectCurveMap)
	{
		Names.emplace_back(Pair.first);
	}
}
