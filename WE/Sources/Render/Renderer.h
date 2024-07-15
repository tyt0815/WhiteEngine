#pragma once

/////////////////////////////////////////////////////////////////////////////////
//#include "DXHeaders.h"
#include "DirectX12/DXUtility.h"
#include "Windows/Window.h"
/////////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <vector>
#include "UploadBuffer.h"
#include "MeshGeometry.h"

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
		int CmdShow = 10, UINT Width = 800, UINT Height = 600,
		DWORD Style = WS_OVERLAPPEDWINDOW
	) override;

protected:
	virtual void ProcessEvent() override;
	virtual void ResizeWindow() override { if (D3D12Device) Resize(); }
	virtual void ClickMouse(WPARAM Button, int X, int Y) override;
	virtual void ReleaseMouse(WPARAM Button, int X, int Y) override;
	virtual void MoveMouse(WPARAM Button, int X, int Y) override;
	virtual void PressKey(WPARAM Key) override;
	virtual void Resize();
	virtual void Update();
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
	void BuildPipelineState();
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
	ComPtr<ID3D12GraphicsCommandList> D3D12CommandList = nullptr;
	ComPtr<IDXGISwapChain> DXGISwapChain = nullptr;
	ComPtr<ID3D12DescriptorHeap> D3D12RTVHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> D3D12DSVHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> D3D12CBVHeap = nullptr;
	ComPtr<ID3D12RootSignature> RootSignature;
	ComPtr<ID3D12PipelineState> Pipelinestate;
	static const int SwapChainBufferCount = 2;
	ComPtr<ID3D12Resource> SwapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource> DepthStencilBuffer;
	ComPtr<ID3DBlob> VSByteCode;
	ComPtr<ID3DBlob> PSByteCode;
	std::unique_ptr<FUploadBuffer<FWVPConstantBuffer>> WVPConstantBuffer;
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
	std::unique_ptr<FMeshGeometry> MeshGeometry;
	

	UINT64 CurrentFence = 0;
	UINT MSAAQuality = 0;
	DXGI_FORMAT DXGIBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT DXGIDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	D3D12_VIEWPORT D3D12Viewport = D3D12_VIEWPORT();
	D3D12_RECT D3D12ScissorRect = D3D12_RECT();

	int CurrBackBuffer = 0;
	bool MSAAState = false;    // 4X MSAA enabled
};

