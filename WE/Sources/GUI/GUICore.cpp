#include "GUICore.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "DirectX/CBVSRVUAVHeap.h"

namespace GUI
{
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

		//////////////////////////////////
		// TestCode
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
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "WhiteEngine Debug"); // 노란색 제목
			ImGui::Separator();

			// FPS 정보 (ImGui 기본 제공)
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

			// 우리가 보려고 하는 애니메이션 프레임 값
			ImGui::Text("Current Frame: %.2f", 1);
		}
		ImGui::End();
		// TestCode
		//////////////////////////////////
		

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