#include "ObjectAnimDataAsset.h"
#include <lz4.h>
#include <tinyxml2.h>
#include <filesystem>
#include "Utility/String.h"
#include "AssetLoader.h"
#include "GUI/GUICore.h"
#include "Utility/Timer.h"

namespace fs = std::filesystem;

bool CompileXMLToOADLz4(const std::string& XMLPath, const std::string& OADLz4Path)
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
	tinyxml2::XMLElement* CurvesNode = Root->FirstChildElement("AnimationCurves");
	if (!InfoNode || !CurvesNode)
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

	// 1. Curve 개수
	int CurveCount = 0;
	for (auto* c = CurvesNode->FirstChildElement("Curve"); c; c = c->NextSiblingElement("Curve")) CurveCount++;
	Append(&CurveCount, sizeof(int));

	// 2. Curve 데이터 순회
	for (auto* curve = CurvesNode->FirstChildElement("Curve"); curve; curve = curve->NextSiblingElement("Curve"))
	{
		std::string name = curve->Attribute("name");
		int nameLen = (int)name.length();
		Append(&nameLen, sizeof(int));
		Append(name.c_str(), nameLen);

		int keyCount = 0;
		for (auto* k = curve->FirstChildElement("Keyframe"); k; k = k->NextSiblingElement("Keyframe")) keyCount++;
		Append(&keyCount, sizeof(int));

		int i = 0;
		std::vector<float> Frames(keyCount);
		std::vector<FKeyframeData> Datas(keyCount);
		for (auto* kp = curve->FirstChildElement("Keyframe"); kp; kp = kp->NextSiblingElement("Keyframe"), ++i)
		{
			FKeyframeData& Data = Datas[i];
			float& Frame = Frames[i];
			Frame = kp->FloatAttribute("frame") - fStart;
			Data.Value = kp->FloatAttribute("value");

			const char* interp = kp->Attribute("interp");
			if (strcmp(interp, "LINEAR") == 0) Data.Interpolation = EInterpolationType::EIT_Linear;
			else if (strcmp(interp, "BEZIER") == 0) Data.Interpolation = EInterpolationType::EIT_Bezier;
			else if (strcmp(interp, "CONSTANT") == 0) Data.Interpolation = EInterpolationType::EIT_Constant;
			else Data.Interpolation = EInterpolationType::EIT_Undefined;

			Data.RightHandle.x = kp->FloatAttribute("h_right_x") - fStart;
			Data.RightHandle.y = kp->FloatAttribute("h_right_y");
			Data.LeftHandle.x = kp->FloatAttribute("h_left_x") - fStart;
			Data.LeftHandle.y = kp->FloatAttribute("h_left_y");
		}
		Append(Frames.data(), sizeof(float) * keyCount);
		Append(Datas.data(), sizeof(FKeyframeData) * keyCount);
	}

	// 3. LZ4 압축
	int originalSize = (int)Payload.size();
	int maxBound = LZ4_compressBound(originalSize);
	std::vector<char> compressed(maxBound);
	int compressedSize = LZ4_compress_default(Payload.data(), compressed.data(), originalSize, maxBound);

	// 4. 저장 (Header: originalSize + Data: compressed)
	std::ofstream outFile(AnsiToWString(OADLz4Path), std::ios::binary);
	if (outFile.is_open())
	{
		outFile.write((char*)&originalSize, sizeof(int));
		outFile.write(compressed.data(), compressedSize);
		outFile.close();
		OutputDebugStringA(("OAD Compile Success: " + OADLz4Path + "\n").c_str());
	}

	return true;
}

bool FObjectAnimDataAsset::LoadAsset(const std::wstring& FilePath)
{
	// 1. 경로 설정
	std::string xmlPath = WStringToString(FilePath);
	std::string lz4Path = xmlPath.substr(0, xmlPath.find_last_of('.')) + ".oad.lz4";

	bool bNeedCompile = false;

	// 2. 파일 존재 여부 및 업데이트 시간 체크
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

	// 3. 필요 시 컴파일 수행
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

	// 4. LZ4 파일로부터 데이터 로드
	if (!Asset::LoadLZ4(lz4Path, mRawBuffer))
	{
		return false;
	}

	unsigned char* Ptr = mRawBuffer.data();

	// 프레임 정보 읽기
	mFPS = *reinterpret_cast<float*>(Ptr);
	Ptr += sizeof(float);
	mFrameEnd = *reinterpret_cast<float*> (Ptr);
	Ptr += sizeof(float);

	// 2. 전체 커브 개수 읽기
	int TotalCurveNum = *reinterpret_cast<int*>(Ptr);
	Ptr += sizeof(int);

	mCurvesStartPtr = Ptr;

	for (int i = 0; i < TotalCurveNum; ++i)
	{
		FCurveInfo CurveInfo;
		CurveInfo.StartPtr = Ptr;

		// 3. 커브 이름 읽기
		int CurveNameLen = *reinterpret_cast<int*>(Ptr);
		Ptr += sizeof(int);

		std::string CurveName(reinterpret_cast<char*>(Ptr), CurveNameLen);
		Ptr += CurveNameLen;

		// 4. 키프레임 개수 읽기
		CurveInfo.TotalKeyFrameNum = *reinterpret_cast<int*>(Ptr);
		Ptr += sizeof(int);

		// 5. 키프레임 데이터 통째로 로드
		size_t DataSize = sizeof(float) * CurveInfo.TotalKeyFrameNum;
		CurveInfo.FramesPtr = reinterpret_cast<float*>(Ptr);
		Ptr += DataSize;

		DataSize = sizeof(FKeyframeData) * CurveInfo.TotalKeyFrameNum;
		CurveInfo.KeyframeDatasPtr = reinterpret_cast<FKeyframeData*>(Ptr);
		Ptr += DataSize;

		mCurveInfoMap[CurveName] = CurveInfo;
	}

	return true;
}