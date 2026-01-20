#include "GUICore.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "DirectX/CBVSRVUAVHeap.h"
#include <vector>
#include <sstream>

inline constexpr float NOTIFICATION_LIFESPAN = 5.0f;
inline constexpr float NOTIFICATION_SIZE_X = 250;
inline constexpr float NOTIFICATION_SIZE_Y = 100;

namespace GUI
{
	std::vector<FDrawCommand> g_DrawCommandQueue;

	std::vector<std::pair<FNotificationDrawCommand, float>> g_NotificationDrawCommandQueue;

	UINT64 AddDrawCommand(const FDrawCommand& Command)
	{
		g_DrawCommandQueue.emplace_back(Command);
		g_DrawCommandQueue.back().ID = g_DrawCommandQueue.size() - 1;
		return g_DrawCommandQueue.back().ID;
	}

	UINT64 AddNotificationDrawCommand(const FNotificationDrawCommand& Command)
	{
		g_NotificationDrawCommandQueue.push_back({ Command, NOTIFICATION_LIFESPAN });
		g_NotificationDrawCommandQueue.back().first.ID = g_NotificationDrawCommandQueue.size() - 1;
		return g_NotificationDrawCommandQueue.back().first.ID;
	}

	void Initialize(
		HWND hWnd,
		ID3D12Device* Device, ID3D12CommandQueue* CommandQueue, DXGI_FORMAT RTVFormat
	)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& IO = ImGui::GetIO();
		IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		IO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		ImGui_ImplDX12_InitInfo InitInfo = {};
		InitInfo.Device = Device;
		InitInfo.CommandQueue = CommandQueue;
		InitInfo.NumFramesInFlight = 3;
		InitInfo.RTVFormat = RTVFormat;

		int ImGuiIndex = GetCBVSRVUAVHeap()->CreateEmptyTexture2DSRV();
		InitInfo.SrvDescriptorHeap = GetCBVSRVUAVHeap()->Get();
		InitInfo.LegacySingleSrvCpuDescriptor = GetCBVSRVUAVHeap()->GetTexture2DCPUDescriptorHandle(ImGuiIndex);
		InitInfo.LegacySingleSrvGpuDescriptor = GetCBVSRVUAVHeap()->GetTexture2DGPUDescriptorHandle(ImGuiIndex);

		ImGui_ImplDX12_Init(&InitInfo);
		ImGui_ImplWin32_Init(hWnd);

	}
	void Update(ID3D12GraphicsCommandList* CommandList)
	{
		// 원래는 메인 루프 시작할때 추가하라고 한 코드
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		float DeltaTime = ImGui::GetIO().DeltaTime;

		for (int i = 0; i < g_DrawCommandQueue.size(); ++i)
		{
			FDrawCommand& DrawCommand = g_DrawCommandQueue[i];
			if (DrawCommand.LifeSpan < 0 || DrawCommand.DrawLambda == nullptr)
			{
				g_DrawCommandQueue[i] = g_DrawCommandQueue.back();
				g_DrawCommandQueue[i].ID = i;
				g_DrawCommandQueue.pop_back();
				--i;
			}
			else
			{
				DrawCommand.DrawLambda();
				g_DrawCommandQueue[i].LifeSpan -= DeltaTime;
			}
		}

		// Notification Rendering
		{
			// LifeSpan이 0이하인 경우, 큐에서 제거
			g_NotificationDrawCommandQueue.erase(
				std::remove_if(
					g_NotificationDrawCommandQueue.begin(), g_NotificationDrawCommandQueue.end(),
					[](const std::pair<FNotificationDrawCommand, float>& Command)
					{ return Command.second < 0;  }
				),
				g_NotificationDrawCommandQueue.end()
			);


			ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoFocusOnAppearing |
				ImGuiWindowFlags_NoNav |
				ImGuiWindowFlags_NoMove;

			ImGuiViewport* Viewport = ImGui::GetMainViewport();

			ImGui::SetNextWindowBgAlpha(1.0f);
			
			for (int i = 0; i < g_NotificationDrawCommandQueue.size(); ++i)
			{
				FNotificationDrawCommand& Command = g_NotificationDrawCommandQueue[i].first;
				g_NotificationDrawCommandQueue[i].second -= DeltaTime;
				std::stringstream Name;
				Name << "Notification_" << i;
				ImVec2 Pos(Viewport->Size.x - NOTIFICATION_SIZE_X, Viewport->Size.y - NOTIFICATION_SIZE_Y * (i + 1));
				ImGui::SetNextWindowPos(Pos, ImGuiCond_Always);
				ImGui::SetNextWindowSize(ImVec2(NOTIFICATION_SIZE_X, NOTIFICATION_SIZE_Y));
				if (ImGui::Begin(Name.str().c_str(), nullptr, WindowFlags))
				{
					Command.DrawLambda();
				}
				ImGui::End();
			}
		}


		

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), CommandList);
	}
	void Shutdown()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}