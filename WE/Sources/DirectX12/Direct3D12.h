#pragma once

#include "Direct3D12Util.h"
#include "Windows/Window.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class UTimer;

class FDirect3D12
{
public:
	FDirect3D12();
	~FDirect3D12();

	virtual bool Initialize(FWindow* TargetWindow);
	virtual int Run();

protected:
	void Resize();
	void FlushCommandQueue();
	ID3D12Resource* CurrentBackBuffer();
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView()const;

	FWindow* Window = nullptr;
	Microsoft::WRL::ComPtr<IDXGIFactory4> DXGIFactory = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> D3D12Device = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Fence> D3D12Fence = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> D3D12CommandQueue = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> D3D12CommandListAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> D3D12GraphicsCommandList = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain> DXGISwapChain = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> D3D12RTVHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> D3D12DSVHeap = nullptr;
	static const int SwapChainBufferCount = 2;
	Microsoft::WRL::ComPtr<ID3D12Resource> D3D12SwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> D3D12DepthStencilBuffer;

	UINT64 CurrentFence = 0;
	UINT RTVDescriptorSize = 0;		// RenderTargetView Descriptor Size
	UINT DSVDescriptorSize = 0;		// DepthStencilView Descriptor Size
	UINT CBVSRVUAVDescriptorSize = 0;	// ConstantBufferView, ShaderResourceView, UnorderedAccessView Descriptor Size
	UINT MSAAQuality = 0;
	DXGI_FORMAT DXGIBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT DXGIDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	D3D12_VIEWPORT D3D12Viewport = D3D12_VIEWPORT();
	D3D12_RECT D3D12ScissorRect = D3D12_RECT();

	int CurrBackBuffer = 0;
	int ClientWidth = 800;
	int ClientHeight = 600;
	bool MSAAState = false;    // 4X MSAA enabled

private:
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* Adapter);
	void LogOutputDisplayModes(IDXGIOutput* Output, DXGI_FORMAT Format);
	void CreateCommandObjects();
	void CreateSwapChain();
	virtual void CreateRTVAndDSVDescriptorHeaps();
	void CalculateFrameStats();
	void Update(UTimer* GameTimer);
	void Draw(UTimer* GameTimer);

	UTimer* mTimer;
	bool mAppPaused = false;

	inline D3D12_RESOURCE_BARRIER CreateResoureceBarrierTransition(
		ID3D12Resource* Resource,
		D3D12_RESOURCE_STATES StateBefore,
		D3D12_RESOURCE_STATES StateAfter,
		UINT Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE
	);
};
