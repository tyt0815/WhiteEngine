#include "GUICore.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "DirectX/CBVSRVUAVHeap.h"
#include <vector>

namespace GUI
{
	std::vector<FDrawCommand> g_DrawCommandQueue;

	UINT64 AddDrawCommand(const FDrawCommand& Command)
	{
		g_DrawCommandQueue.emplace_back(Command);
		g_DrawCommandQueue.back().ID = g_DrawCommandQueue.size() - 1;
		return g_DrawCommandQueue.back().ID;
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
				g_DrawCommandQueue[i].LifeSpan -= ImGui::GetIO().DeltaTime;
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