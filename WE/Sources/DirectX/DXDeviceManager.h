#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

#include "Utility/d3dx12.h"
#include "Utility/Class.h"
#include "Utility/DXUtility.h"

constexpr int SWAPCHAIN_BUFFERS_NUM = 3;

class FWindow;

class FDXDeviceManager
{
	SINGLETON(FDXDeviceManager);
public:
	bool Initialize(FWindow* Window);
	void Resize(UINT Width, UINT Height);
	void FlushCommandQueue();

	// TODO
	class WViewCamera* Camera = nullptr;

private:
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* Adapter);
	void LogOutputDisplayModes(IDXGIOutput* Output, DXGI_FORMAT Format);
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateDescriptorHeaps();

	Microsoft::WRL::ComPtr<IDXGIFactory4> Factory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> Device;
	Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RTVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DSVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> SwapChainBuffers[SWAPCHAIN_BUFFERS_NUM];
	Microsoft::WRL::ComPtr<ID3D12Resource> DepthStencilBuffer;
	FWindow* mWindow = nullptr;
	UINT64 CurrentFence = 0;

	D3D12_VIEWPORT ScreenViewport = {};
	D3D12_RECT ScissorRect = {};
	DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	UINT RTVDescriptorSize = 0;
	UINT DSVDescriptorSize = 0;
	UINT CBVSRVUAVDescriptorSize = 0;
	UINT MSAAQuality_4x = 0;
	UINT CurrentBackBuffer = 0;

	bool bMSAA = false;
public:
	inline D3D12_VIEWPORT GetScreenViewport() const
	{
		return ScreenViewport;
	}
	inline D3D12_RECT GetScissorRect() const
	{
		return ScissorRect;
	}
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView()
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			RTVHeap->GetCPUDescriptorHandleForHeapStart(),
			CurrentBackBuffer,
			RTVDescriptorSize
		);
	}
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const 
	{ 
		return DSVHeap->GetCPUDescriptorHandleForHeapStart();
	}
	inline ID3D12Resource* GetCurrentBackBufferPtr() const
	{
		return SwapChainBuffers[CurrentBackBuffer].Get();
	}
	inline ID3D12Device* GetDevicePtr() const 
	{ 
		return Device.Get(); 
	}
	inline ID3D12CommandQueue* GetCommandQueuePtr() const
	{ 
		return CommandQueue.Get();
	}
	inline ID3D12CommandAllocator* GetCommandAllocatorPtr() const 
	{
		return CommandAllocator.Get(); 
	}
	inline ID3D12GraphicsCommandList* GetCommandListPtr() const 
	{ 
		return CommandList.Get(); 
	}
	inline float GetAspectRatio() const
	{
		return ScreenViewport.Width / ScreenViewport.Height;
	}
	inline UINT GetCBVSRVUAVDescriptorSize() const
	{
		return CBVSRVUAVDescriptorSize;
	}
	inline DXGI_FORMAT GetBackbufferFormat() const
	{
		return BackBufferFormat;
	}
	inline DXGI_FORMAT GetDepthStencilFormat() const
	{
		return DepthStencilFormat;
	}
	inline UINT GetMSAAQuality_4x() const
	{
		return MSAAQuality_4x;
	}
	inline bool IsMSAAOn() const
	{
		return bMSAA;
	}
	inline ID3D12Fence* GetFencePtr() const
	{
		return Fence.Get();
	}
	inline void PresentAndSwapBuffer()
	{
		ThrowIfFailed(SwapChain->Present(0, 0));
		CurrentBackBuffer = (CurrentBackBuffer + 1) % SWAPCHAIN_BUFFERS_NUM;
	}
	inline void SignalFence()
	{
		CommandQueue->Signal(Fence.Get(), ++CurrentFence);
	}
	inline UINT64 GetCurrentFence() const
	{
		return CurrentFence;
	}
};