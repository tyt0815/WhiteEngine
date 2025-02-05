#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "DXUtility.h"
#include "Utility/Class.h"
#include "Utility/Timer.h"

constexpr const WCHAR* DXWindowTitle = L"WhiteEngine";
constexpr const WCHAR* DXWindowClassName = L"DXWindow";
constexpr UINT MaxTitle = 256;
constexpr UINT DXWidth = 800;
constexpr UINT DXHeight = 600;
constexpr UINT SwapChainBufferCount = 2;

inline HINSTANCE GetHInstance() { return GetModuleHandle(nullptr); }

class FDXWindow
{
	SINGLETON(FDXWindow);
public:
	struct FCamera
	{
		float Theta = 0;
		float Phi = 0;
		float Radius = 0;
		XMFLOAT3 EyePos;
		XMFLOAT4X4 Project;
	};
	FCamera Camera;
	float MaxCameraRadius = 30.0f;
	float MinCameraRadius = 5.0f;
	bool Initialize();
	bool InitializeWindow();
	bool InitializeDirectX();
	void ResetCameraPosition();
	int Run();
	virtual LRESULT InternalWndProc(HWND HWnd, UINT Message, WPARAM WParam, LPARAM LParam);
	RECT GetClientRect();
	inline UINT GetClientWidth() { return GetClientRect().right - GetClientRect().left; }
	inline UINT GetClientHeight() { return GetClientRect().bottom - GetClientRect().top; }
	inline float GetAspectRatio() { return static_cast<float>(GetClientWidth()) / GetClientHeight(); }
	inline UINT GetRTVDescriptorSize() { return RTVDescriptorSize; }
	inline UINT GetDSVDescriptorSize() { return DSVDescriptorSize; }
	inline UINT GetCBVSRVUAVDescriptorSize() { return CBVSRVUAVDescriptorSize; }
	inline IDXGISwapChain* GetSwapChain() { return SwapChain.Get(); }
	inline ID3D12Device* GetDevice() { return Device.Get(); }
	inline ID3D12CommandQueue* GetCommandQueue() { return CommandQueue.Get(); }
	inline ID3D12CommandAllocator* GetCommandAllocator() { return CommandAllocator.Get(); }
	inline ID3D12GraphicsCommandList* GetCommandList() { return CommandList.Get(); }
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView()
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			RTVHeap->GetCPUDescriptorHandleForHeapStart(),
			CurrentBackBuffer,
			RTVDescriptorSize
		);
	}
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() { return DSVHeap->GetCPUDescriptorHandleForHeapStart(); }
	inline ID3D12Resource* GetCurrentBackBuffer() const { return SwapChainBuffer[CurrentBackBuffer].Get(); }
	inline DXGI_FORMAT GetBackBufferFormat() const { return BackBufferFormat; }
	inline DXGI_FORMAT GetDepthStencilFormat() const { return DepthStencilFormat; }
	inline ID3D12Fence* GetFence() const { return Fence.Get(); }
	wstring GetWindowTitle();
	inline bool Get4xMSAAState() const { return bMSAA; }
	inline UINT Get4xMSAAQuality() const { return MSAAQuality_4x; }
	inline const UTimer* GetMainTimer() const { return &Timer; }
	inline D3D12_VIEWPORT GetViewport() const { return ScreenViewport; }
	inline D3D12_RECT GetScissorRect() const { return ScissorRect; }
	inline void Set4xMsaaState(bool Value) { bMSAA = Value; }
	inline ID3D12Fence* GetFence() { return Fence.Get(); }
	inline UINT64 GetCurrentFence() { return CurrentFence; }
	// Add 1 to CurrentFence and return CurrentFence
	inline UINT64 GetNextFence() { return ++CurrentFence; }
	void FlushCommandQueue();
	void ExecuteCommand();
	void SwapRenderTargetBuffers();

private:
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* Adapter);
	void LogOutputDisplayModes(IDXGIOutput* Output, DXGI_FORMAT Format);
	void CalculateFrameStats();
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateDescriptorHeaps();
	void Resize();
	void OnMouseDown(WPARAM WParam, int X, int Y);
	void OnMouseUp(WPARAM WParam, int X, int Y);
	void OnMouseMove(WPARAM WParam, int X, int Y);
	void OnKeyDown(WPARAM WParam);
	void OnKeyUp(WPARAM WParam);
	void OnMouseWheel(WPARAM WParam);
	void ProcessInput();

	UTimer Timer;
	HWND HWnd;
	ComPtr<IDXGIFactory4> Factory;
	ComPtr<IDXGISwapChain> SwapChain;
	ComPtr<ID3D12Device> Device;
	ComPtr<ID3D12Fence> Fence;
	ComPtr<ID3D12CommandQueue> CommandQueue;
	ComPtr<ID3D12CommandAllocator> CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> CommandList;
	ComPtr<ID3D12DescriptorHeap> RTVHeap;
	ComPtr<ID3D12DescriptorHeap> DSVHeap;
	ComPtr<ID3D12Resource> SwapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource> DepthStencilBuffer;
	DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;
	UINT64 CurrentFence = 0;
	UINT RTVDescriptorSize = 0;
	UINT DSVDescriptorSize = 0;
	UINT CBVSRVUAVDescriptorSize = 0;
	UINT MSAAQuality_4x = 0;
	UINT CurrentBackBuffer = 0;
	POINT LastMousePos;
	bool bMSAA = false;
	bool bAppPaused = false;
	bool bMinimized = false;
	bool bMaximized = false;
	bool bResized = false;
};

inline bool IsKeyPressed(char Key) { return GetAsyncKeyState(Key) & 0x8000; }