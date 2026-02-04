#include "AssetManager.h"
#include <filesystem>
#include "SplineDataAsset.h"
#include "XMLDataAsset.h"
#include "ObjectAnimDataAsset.h"
#include "BlueprintAsset.h"
#include "Utility/Container.h"
#include "Utility/String.h"

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
	LoadSplines();

	LoadBlueprints();

	LoadObjectAnimations();
}

FAsset* FAssetManager::GetAsset(const std::string& Name)
{
	if (mAssets.find(Name) == mAssets.end())
	{
		return nullptr;
	}

	return mAssets[Name].get();
}

void FAssetManager::LoadBlueprints()
{
	const std::wstring BlueprintsPath = SOLUTION_DIR_W + std::wstring(L"Resources\\Blueprints");
	const std::wstring Extension = L".bp";

	// Actor Blueprints
	{
		TArray<std::wstring> BlueprintFileName;
		const std::wstring FolderPath = BlueprintsPath + L"\\Actors";
		GetFileNameByExtension(FolderPath, Extension, BlueprintFileName);

		for (const auto& FileName : BlueprintFileName)
		{
			LoadAsset<FActorBlueprintAsset>(FolderPath + L"\\" + FileName + Extension, WStringToString(FileName));
		}
	}

	// Component Blueprints
	{
		TArray<std::wstring> BlueprintFileName;
		const std::wstring FolderPath = BlueprintsPath + L"\\Components";
		GetFileNameByExtension(FolderPath, Extension, BlueprintFileName);

		for (const auto& FileName : BlueprintFileName)
		{
			LoadAsset<FComponentBlueprintAsset>(FolderPath + L"\\" + FileName + Extension, WStringToString(FileName));
		}
	}
	
}

void FAssetManager::LoadObjectAnimations()
{
	const std::wstring FolderPath = SOLUTION_DIR_W + std::wstring(L"Resources\\ObjectAnimations");
	const std::wstring Extension = L".oa";
	TArray<std::wstring> ObjAnimFileName;

	GetFileNameByExtension(FolderPath, Extension, ObjAnimFileName);

	for (const auto& FileName : ObjAnimFileName)
	{
		LoadAsset<FObjectAnimDataAsset>(FolderPath + L"\\" + FileName + Extension, WStringToString(FileName));
	}
}

void FAssetManager::LoadSplines()
{
	const std::wstring FolderPath = SOLUTION_DIR_W + std::wstring(L"Resources\\Splines");
	const std::wstring Extension = L".json";
	TArray<std::wstring> SplineFileName;

	GetFileNameByExtension(FolderPath, Extension, SplineFileName);

	for (const auto& FileName : SplineFileName)
	{
		LoadAsset<FSplineDataAsset>(FolderPath + L"\\" + FileName + Extension, WStringToString(FileName));
	}
}
