#pragma once
#include <Windows.h>
#include <d3d12.h>

namespace GUI
{
	struct FDrawCommand
	{
		UINT64 ID;
		void (*DrawLambda)();
		float LifeSpan;		// 0: 1프레임 그리기
	};

	UINT64 AddDrawCommand(const FDrawCommand& Command);

	void Initialize(
		HWND hWnd,
		ID3D12Device* Device, ID3D12CommandQueue* CommandQueue, DXGI_FORMAT RTVFormat
	);

	void Update(ID3D12GraphicsCommandList* CommandList);

	void Shutdown();
}