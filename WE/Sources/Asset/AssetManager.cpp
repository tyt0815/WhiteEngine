#include "AssetManager.h"
#include <filesystem>
#include "SplineDataAsset.h"
#include "XMLDataAsset.h"
#include "ObjectAnimDataAsset.h"
#include "BlueprintAsset.h"
#include "Utility/Container.h"

void GetFileNameByExtension(const std::wstring& InPath, const std::wstring& InExtension, TArray<std::wstring>& OutFileNames)
{
	std::filesystem::path TargetPath(InPath);

	if (!std::filesystem::exists(TargetPath) || !std::filesystem::is_directory(TargetPath))
	{
		return;
	}

	// 점(.)이 없는 확장자가 들어왔을 경우를 대비한 안전장치
	std::wstring Ext = InExtension.front() == L'.' ? InExtension : L"." + InExtension;

	for (const auto& Entry : std::filesystem::directory_iterator(TargetPath))
	{
		if (Entry.is_regular_file() && Entry.path().extension() == Ext)
		{
			OutFileNames.push_back(Entry.path().stem().wstring());
		}
	}
}

FAssetManager::FAssetManager()
{
	
}

FAssetManager::~FAssetManager()
{
}

void FAssetManager::LoadAssets()
{
	std::wstring SolutionDir(SOLUTION_DIR_W);
	LoadAsset<FSplineDataAsset>(SolutionDir + L"Resources/JSON/SplineData.json", L"SDA_ProjectilePath");
	LoadAsset<FSplineDataAsset>(SolutionDir + L"Resources/JSON/ForwardSpline.json", L"SDA_Forward");
	LoadAsset<FSplineDataAsset>(SolutionDir + L"Resources/JSON/RightSpline.json", L"SDA_Right");
	LoadAsset<FSplineDataAsset>(SolutionDir + L"Resources/JSON/UpSpline.json", L"SDA_Up");

	std::wstring JsonDir = SolutionDir + L"Resources/JSON";
	LoadAsset<FSplineDataAsset>(JsonDir + L"/SpiralSpline.json", L"SDA_Spiral");
	LoadAsset<FSplineDataAsset>(JsonDir + L"/WaveSpline.json", L"SDA_Wave");
	LoadAsset<FSplineDataAsset>(JsonDir + L"/RingPathSpline.json", L"SDA_RingPath");

	std::wstring XMLDir = SolutionDir + L"Resources/XML";	
	LoadAsset<FObjectAnimDataAsset>(XMLDir + L"/MultiAnim.xml", L"OAD_MultiAnim");
	LoadAsset<FObjectAnimDataAsset>(XMLDir + L"/LeftRight.xml", L"OAD_LeftRight");
	LoadAsset<FObjectAnimDataAsset>(XMLDir + L"/ColdLaunch.xml", L"OAD_ColdLaunch");
	LoadAsset<FObjectAnimDataAsset>(XMLDir + L"/MissileTrack.xml", L"OAD_MissileTrack");

	LoadBlueprints();
}

FAsset* FAssetManager::GetAsset(const std::wstring& Name)
{
	if (mAssets.find(Name) == mAssets.end())
	{
		return nullptr;
	}

	return mAssets[Name].get();
}

void FAssetManager::LoadBlueprints()
{
	const std::wstring FolderPath = SOLUTION_DIR_W + std::wstring(L"Resources\\Blueprints");
	const std::wstring Extension = L".bp";
	TArray<std::wstring> BlueprintFileName;

	GetFileNameByExtension(FolderPath, Extension, BlueprintFileName);

	const std::wstring Prefix = L"BP_";
	for (const auto& FileName : BlueprintFileName)
	{
		LoadAsset<FBlueprintAsset>(FolderPath + L"\\" + FileName + Extension, Prefix + FileName);
	}
}
