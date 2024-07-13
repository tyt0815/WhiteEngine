#pragma once

/////////////////////////////////////////////////////////////////////////////////
#include "DXHeaders.h"
#include "Windows/Window.h"
/////////////////////////////////////////////////////////////////////////////////

#include <memory>
#include "UploadBuffer.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class FRenderer : public FWindow
{
	typedef FWindow Super;
public:
	FRenderer() {};
	~FRenderer() {};
	virtual bool Initialize(const WCHAR* Title = L"DefaultTitle",
		int CmdShow = 10, UINT Width = 1920, UINT Height = 1080,
		DWORD Style = WS_OVERLAPPED | WS_SYSMENU
	) override;

protected:
	virtual void ResizeWindow() override { if (D3D12Device) Resize(); }
	virtual void Resize();
	virtual void Event() override;
	virtual void Render();
	virtual void StartRendering();
	virtual void EndRendering();
	void InitializeCommand();
	void ExecuteCommand();
	void FlushCommandQueue();
	inline ID3D12Resource* GetCurrentBackBuffer();
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
		ID3D12DescriptorHeap* DescriptorHeap,
		UINT DescriptorSize,
		UINT i
	) const;
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(UINT i) const { return GetCPUDescriptorHandle(D3D12RTVHeap.Get(), GetRTVDescriptorSize(), i); }
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView() const { return GetRTVHandle(CurrBackBuffer); }
	inline D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const { return D3D12DSVHeap->GetCPUDescriptorHandleForHeapStart(); }
	UINT GetRTVDescriptorSize() const;
	UINT GetDSVDescriptorSize() const;
	UINT GetCBVSRVUAVDescriptorSize() const;
	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* Adapter);
	void LogOutputDisplayModes(IDXGIOutput* Output, DXGI_FORMAT Format);
	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateDescriptorHeaps();
	D3D12_RESOURCE_BARRIER CreateResoureceBarrierTransition(
		ID3D12Resource* Resource,
		D3D12_RESOURCE_STATES StateBefore,
		D3D12_RESOURCE_STATES StateAfter,
		UINT Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		D3D12_RESOURCE_BARRIER_FLAGS Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE
	);
	void BuildConstantBuffer();
	void BuildRootSignature();
	void BuildShaderAndLayout();
	ComPtr<ID3DBlob> CompileShader(
		const std::wstring& FileName,
		const D3D_SHADER_MACRO* Defines,
		const std::string& EntryPoint,
		const std::string& Target
	);

	ComPtr<IDXGIFactory4> DXGIFactory = nullptr;
	ComPtr<ID3D12Device> D3D12Device = nullptr;
	ComPtr<ID3D12Fence> D3D12Fence = nullptr;
	ComPtr<ID3D12CommandQueue> D3D12CommandQueue = nullptr;
	ComPtr<ID3D12CommandAllocator> D3D12CommandListAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> D3D12GraphicsCommandList = nullptr;
	ComPtr<IDXGISwapChain> DXGISwapChain = nullptr;
	ComPtr<ID3D12DescriptorHeap> D3D12RTVHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> D3D12DSVHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> D3D12CBVHeap = nullptr;
	ComPtr<ID3D12RootSignature> RootSignature;
	static const int SwapChainBufferCount = 2;
	ComPtr<ID3D12Resource> D3D12SwapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource> D3D12DepthStencilBuffer;
	ComPtr<ID3DBlob> UnlitVSByteCode;
	ComPtr<ID3DBlob> UnlitPSByteCode;
	std::unique_ptr<FUploadBuffer<FWVPConstantBuffer>> WVPConstantBuffer;
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
	

	UINT64 CurrentFence = 0;
	UINT MSAAQuality = 0;
	DXGI_FORMAT DXGIBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT DXGIDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	D3D12_VIEWPORT D3D12Viewport = D3D12_VIEWPORT();
	D3D12_RECT D3D12ScissorRect = D3D12_RECT();

	int CurrBackBuffer = 0;
	bool MSAAState = false;    // 4X MSAA enabled
};

