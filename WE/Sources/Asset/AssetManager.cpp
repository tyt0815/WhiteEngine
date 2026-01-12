#include "AssetManager.h"
#include "SplineDataAsset.h"

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
}

FAsset* FAssetManager::GetAsset(const std::wstring& Name)
{
	if (mAssets.find(Name) == mAssets.end())
	{
		return nullptr;
	}

	return mAssets[Name].get();
}