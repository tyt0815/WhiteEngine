#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <functional>
#include "imgui.h"

namespace GUI
{
	struct FDrawCommand
	{
		UINT64 ID;
		std::function<void()> DrawLambda;
		float LifeSpan;		// 0: 1프레임 그리기
	};

	struct FNotificationDrawCommand
	{
		UINT64 ID;
		std::function<void()> DrawLambda;
	};

	UINT64 AddDrawCommand(const FDrawCommand& Command);

	UINT64 AddNotificationDrawCommand(const FNotificationDrawCommand& Command);

	void Initialize(
		HWND hWnd,
		ID3D12Device* Device, ID3D12CommandQueue* CommandQueue, DXGI_FORMAT RTVFormat
	);

	void Update(ID3D12GraphicsCommandList* CommandList);

	void Shutdown();
}