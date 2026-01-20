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

	
	LoadAsset<FXMLDataAsset>(XMLDir + L"/Test.xml", L"XDA_Test");
	LoadAsset<FXMLDataAsset>(XMLDir + L"/Projectile_Test.xml", L"XDA_Projectile_Test");


	// 대용량 XML 테스트 블록
	{
		const size_t Iterations = 100;
		float TotalTime = 0;
		UTimer Timer;
		Timer.Reset();
		for (int i = 0; i < Iterations; ++i)
		{
			std::wstringstream Name;
			Name << L"XDA_Large_" << i;
			LoadAsset<FXMLDataAsset>(XMLDir + L"/Large.xml", Name.str().c_str());
			Timer.Tick();
			TotalTime += Timer.GetDeltaTime();
		}
		GUI::FNotificationDrawCommand Command;
		Command.DrawLambda = [=]()
		{
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "XML file Load Time"); // 노란색 제목
			ImGui::Separator();

			// FPS 정보 (ImGui 기본 제공)
			ImGui::TextWrapped("Iterations: %d\nTotal Time: %f\nAverage Time: %f", Iterations, TotalTime, TotalTime / Iterations);
		};
		GUI::AddNotificationDrawCommand(Command);
	}
}

FAsset* FAssetManager::GetAsset(const std::wstring& Name)
{
	if (mAssets.find(Name) == mAssets.end())
	{
		return nullptr;
	}

	return mAssets[Name].get();
}