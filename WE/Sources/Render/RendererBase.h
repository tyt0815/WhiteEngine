#pragma once

#include "Utility/DXUtility.h"
#include "FrameResource.h"
#include "Utility/Timer.h"
#include "Runtime/World/World.h"
#include "Texture.h"

class AActor;
class WViewCamera;

constexpr const WCHAR* DXWindowTitle = L"WhiteEngine";
constexpr const WCHAR* DXWindowClassName = L"DXWindow";
constexpr UINT MaxTitle = 256;
constexpr UINT DXWidth = 800;
constexpr UINT DXHeight = 600;
constexpr UINT SwapChainBufferCount = 2;

extern const int NumFrameResources;

inline HINSTANCE GetHInstance() { return GetModuleHandle(nullptr); }

class FRendererBase : FNoncopyable
{
public:
	FRendererBase();
	static FRendererBase* Renderer;

public:
	inline float GetAspectRatio() { return static_cast<float>(GetClientWidth()) / GetClientHeight(); }
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() { return DSVHeap->GetCPUDescriptorHandleForHeapStart(); }
	inline ID3D12Resource* GetCurrentBackBuffer() const { return SwapChainBuffer[CurrentBackBuffer].Get(); }
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView()
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			RTVHeap->GetCPUDescriptorHandleForHeapStart(),
			CurrentBackBuffer,
			RTVDescriptorSize
		);
	}
	virtual bool Initialize();
	int Run();
	virtual LRESULT InternalWndProc(HWND HWnd, UINT Message, WPARAM WParam, LPARAM LParam);

protected:

	wstring GetWindowTitle();
	RECT GetClientRect();
	inline UINT GetClientWidth() { return GetClientRect().right - GetClientRect().left; }
	inline UINT GetClientHeight() { return GetClientRect().bottom - GetClientRect().top; }
	bool InitializeWindow();
	bool InitializeDirectX();
	void SetTargetFrameResource();
	void FlushCommandQueue();
	virtual void Render(UTimer* Timer);
	void BuildFrameResources();
	virtual void BuildDescriptorHeaps() = 0;
	virtual void BuildConstantBuffers() = 0;
	virtual void BuildShaderResources() = 0;
	virtual void BuildRootSignature() = 0;
	virtual void BuildShaderAndInputLayout() = 0;
	virtual void BuildPipelineStateObject() = 0;

	UTimer MainTimer;
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
	unique_ptr<WWorld> World;
	vector<unique_ptr<FFrameResource>> FrameResources;
	unordered_map<string, vector<D3D12_INPUT_ELEMENT_DESC>> InputLayouts;
	unordered_map<string, ComPtr<ID3DBlob>> Shaders;
	vector<ComPtr<ID3D12PipelineState>> PipelineStateObjects;
	FFrameResource* TargetFrameResource = nullptr;
	WViewCamera* Camera = nullptr;
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
	int TargetFrameResourceIndex = 0;
	bool bMSAA = false;
	bool bAppPaused = false;
	bool bMinimized = false;
	bool bMaximized = false;
	bool bResized = false;
	bool bWireFrame = false;

private:
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* Adapter);
	void LogOutputDisplayModes(IDXGIOutput* Output, DXGI_FORMAT Format);
	void CalculateFrameStats();
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateDescriptorHeaps();
	void Set4xMsaaState(bool Value);
	void Resize();
	void OnMouseDown(WPARAM WParam, int X, int Y);
	void OnMouseUp(WPARAM WParam, int X, int Y);
	void OnMouseMove(WPARAM WParam, int X, int Y);
	void OnMouseWheel(WPARAM WParam);
	void OnKeyDown(WPARAM WParam);
	void OnKeyUp(WPARAM WParam);
	void ProcessInput();
};