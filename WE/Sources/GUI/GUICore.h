#pragma once
#include <Windows.h>
#include <d3d12.h>

namespace GUI
{
	void Initialize(
		HWND hWnd,
		ID3D12Device* Device, ID3D12CommandQueue* CommandQueue, DXGI_FORMAT RTVFormat
	);

	void Update(ID3D12GraphicsCommandList* CommandList);

	void Shutdown();
}