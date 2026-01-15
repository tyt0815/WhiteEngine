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
	std::wstring JsonDir = SolutionDir + L"Resources/JSON";

	LoadAsset<FSplineDataAsset>(SolutionDir + L"Resources/JSON/SplineData.json", L"SDA_ProjectilePath");
	LoadAsset<FSplineDataAsset>(SolutionDir + L"Resources/JSON/ForwardSpline.json", L"SDA_Forward");
	LoadAsset<FSplineDataAsset>(SolutionDir + L"Resources/JSON/RightSpline.json", L"SDA_Right");
	LoadAsset<FSplineDataAsset>(SolutionDir + L"Resources/JSON/UpSpline.json", L"SDA_Up");
	LoadAsset<FSplineDataAsset>(JsonDir + L"/SpiralSpline.json", L"SDA_Spiral");
	LoadAsset<FSplineDataAsset>(JsonDir + L"/WaveSpline.json", L"SDA_Wave");
	LoadAsset<FSplineDataAsset>(JsonDir + L"/RingPathSpline.json", L"SDA_RingPath");
}

FAsset* FAssetManager::GetAsset(const std::wstring& Name)
{
	if (mAssets.find(Name) == mAssets.end())
	{
		return nullptr;
	}

	return mAssets[Name].get();
}