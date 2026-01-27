#include "AssetManager.h"
#include "SplineDataAsset.h"
#include "XMLDataAsset.h"
#include "ObjectAnimDataAsset.h"

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
	LoadAsset<FXMLDataAsset>(XMLDir + L"/Test.xml", L"XDA_Test");
	LoadAsset<FXMLDataAsset>(XMLDir + L"/Projectile_Test.xml", L"XDA_Projectile_Test");

	
	LoadAsset<FObjectAnimDataAsset>(XMLDir + L"/MultiAnim.xml", L"OAD_MultiAnim");
	LoadAsset<FObjectAnimDataAsset>(XMLDir + L"/LeftRight.xml", L"OAD_LeftRight");
	LoadAsset<FObjectAnimDataAsset>(XMLDir + L"/ColdLaunch.xml", L"OAD_ColdLaunch");
	LoadAsset<FObjectAnimDataAsset>(XMLDir + L"/MissileTrack.xml", L"OAD_MissileTrack");
}

FAsset* FAssetManager::GetAsset(const std::wstring& Name)
{
	if (mAssets.find(Name) == mAssets.end())
	{
		return nullptr;
	}

	return mAssets[Name].get();
}