#include "AssetManager.h"
#include "SplineDataAsset.h"
#include "XMLDataAsset.h"
#include "GUI/GUICore.h"
#include "Utility/Timer.h"



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

	UTimer Timer;
	Timer.Reset();
	LoadAsset<FXMLDataAsset>(XMLDir + L"/Test.xml", L"XDA_Test");
	Timer.Tick();
	float TestDelta = Timer.GetDeltaTime();
	LoadAsset<FXMLDataAsset>(XMLDir + L"/Projectile_Test.xml", L"XDA_Projectile_Test");
	Timer.Tick();
	float ProjectileTestDelta = Timer.GetDeltaTime();
	LoadAsset<FXMLDataAsset>(XMLDir + L"/Large.xml", L"XDA_Large");
	Timer.Tick();
	float LargeDelta = Timer.GetDeltaTime();

	GUI::FNotificationDrawCommand Command;
	Command.DrawLambda = [=]()
	{
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "XML file Load Time"); // 노란색 제목
		ImGui::Separator();

		// FPS 정보 (ImGui 기본 제공)
		ImGui::TextWrapped("Test.xml: %f\nProjectile_Test.xml: %f\nLarge.xml: %f", TestDelta, ProjectileTestDelta, LargeDelta);
	};
	GUI::AddNotificationDrawCommand(Command);
}

FAsset* FAssetManager::GetAsset(const std::wstring& Name)
{
	if (mAssets.find(Name) == mAssets.end())
	{
		return nullptr;
	}

	return mAssets[Name].get();
}