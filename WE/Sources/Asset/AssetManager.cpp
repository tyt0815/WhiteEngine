#include "AssetManager.h"
#include "SplineDataAsset.h"

FAssetManager::FAssetManager()
{
	// Load Assets
	std::wstring SolutionDir(SOLUTION_DIR_W);

	LoadAsset<FSplineDataAsset>(SolutionDir + L"Resources/JSON/SplineData.json", L"SDA_ProjectilePath");
}

FAssetManager::~FAssetManager()
{
}