#include "DefaultWorld.h"
#include "../Actor/DirectionalLight.h"
#include "../Actor/Floor.h"
#include "../Component/DirectionalLightComponent.h"
#include "../Pawn/GhostCameraPawn.h"
#include "GUI/GUICore.h"

WDefaultWorld::WDefaultWorld()
{
	auto Player = SpawnActor<AGhostCameraPawn>();
	SetPlayer(Player);

	auto LightActor = SpawnActor<ADirectionalLight>().lock();
	LightActor->SetActorRotation(XMFLOAT3(0.0f, -45, -45));
	LightActor->GetDirLightComp().lock()->SetColor({ 10.0f, 10.0f, 10.0f });

	auto Floor = SpawnActor<AFloor>().lock();
	Floor->SetActorLocation(XMFLOAT3(0.0f, -5.0f, 0.0f));
}

void WDefaultWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*static float FPSAverage = 0;
	static int FrameCounter = 0;
	++FrameCounter;
	FPSAverage = */

	GUI::FDrawCommand Command;
	Command.LifeSpan = 0;
	Command.DrawLambda = []()
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

		ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoNav |
			ImGuiWindowFlags_NoMove;

		ImGui::SetNextWindowBgAlpha(1.0f);

		if (ImGui::Begin("DebugOverlay", nullptr, WindowFlags))
		{
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "-----Debug-----"); // 노란색 제목
			ImGui::Separator();

			// FPS 정보 (ImGui 기본 제공)
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		}
		ImGui::End();
	};
	AddDrawCommand(Command);
}
