#include "Render/Renderer.h"
#include <vector>
#include "Utility/Timer.h"
#include "Utility/Math.h"

bool FRenderer::Initialize(const WCHAR* Title, int CmdShow, UINT Width, UINT Height, DWORD Style)
{
	Super::Initialize(Title, CmdShow, Width, Height, Style);
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> DebugController;
		THROWIFFAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController)));
		DebugController->EnableDebugLayer();
	}
#endif

	// Create DXGIFactory
	THROWIFFAILED(CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFactory)));

	// Create Device
	HRESULT HResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&D3D12Device));

	if (FAILED(HResult))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> WarpAdapter;
		THROWIFFAILED(DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter)));

		THROWIFFAILED(
			D3D12CreateDevice(
				WarpAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&D3D12Device)
			)
		);
	}

	// Create Fence
	THROWIFFAILED(D3D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&D3D12Fence)));

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MSAAQualityLevels;
	MSAAQualityLevels.Format = DXGIBackBufferFormat;
	MSAAQualityLevels.SampleCount = 4;
	MSAAQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	MSAAQualityLevels.NumQualityLevels = 0;
	THROWIFFAILED(
		D3D12Device->CheckFeatureSupport(
			D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
			&MSAAQualityLevels,
			sizeof(MSAAQualityLevels)
		)
	);

	MSAAQuality = MSAAQualityLevels.NumQualityLevels;
	// MSAAQuality는 항상 0보다 커야한다.
	assert(MSAAQuality > 0 && "Unexpected MSAA quality level.");

	#ifdef _DEBUG
	LogAdapters();
	#endif

	CreateCommandObjects();
	CreateSwapChain();
	CreateDescriptorHeaps();
	Resize();

	// Reset the command list to prep for initialization commands.
	InitializeCommand();
	BuildConstantBuffer();
	BuildRootSignature();
	BuildShaderAndLayout();
	MeshGeometry = std::make_unique<FMeshGeometry>(BuildBoxMeshGeometry(D3D12Device.Get(), D3D12CommandList.Get()));
	BuildPipelineState();

	ExecuteCommand();

	return true;
}

void FRenderer::ProcessEvent()
{
	Super::ProcessEvent();
	Update();
	Render();
}

void FRenderer::ClickMouse(WPARAM Button, int X, int Y)
{
	Super::ClickMouse(Button, X, Y);
}

void FRenderer::ReleaseMouse(WPARAM Button, int X, int Y)
{
	Super::ReleaseMouse(Button, X, Y);
}

void FRenderer::MoveMouse(WPARAM Button, int X, int Y)
{
	Super::MoveMouse(Button, X, Y);
	if (IsMouseClicked(Button, MK_LBUTTON))
	{
		
	}
	else if (IsMouseClicked(Button, MK_RBUTTON))
	{

	}
	else if (IsMouseClicked(Button, MK_MBUTTON))
	{

	}
}

void FRenderer::PressKey(WPARAM Key)
{
	Super::PressKey(Key);
	if (Key == 'W')
	{

	}
}

void FRenderer::Resize()
{
	assert(D3D12Device);
	assert(DXGISwapChain);
	assert(D3D12CommandListAllocator);

	// Flush before changing any resources.
	FlushCommandQueue();

	InitializeCommand();
	//THROWIFFAILED(D3D12CommandList->Reset(D3D12CommandListAllocator.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < SwapChainBufferCount; ++i)
	{
		SwapChainBuffer[i].Reset();
	}
	DepthStencilBuffer.Reset();

	// Resize the swap chain.
	THROWIFFAILED(DXGISwapChain->ResizeBuffers(
		SwapChainBufferCount,
		GetClientWidth(), GetClientHeight(),
		DXGIBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	CurrBackBuffer = 0;

	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE D3D12RTVHeapHandle = GetRTVHandle(i);
		THROWIFFAILED(DXGISwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffer[i])));
		D3D12Device->CreateRenderTargetView(SwapChainBuffer[i].Get(), nullptr, D3D12RTVHeapHandle);
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC D3D12DepthStencilBufferDesc;
	D3D12DepthStencilBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	D3D12DepthStencilBufferDesc.Alignment = 0;
	D3D12DepthStencilBufferDesc.Width = GetClientWidth();
	D3D12DepthStencilBufferDesc.Height = GetClientHeight();
	D3D12DepthStencilBufferDesc.DepthOrArraySize = 1;
	D3D12DepthStencilBufferDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	D3D12DepthStencilBufferDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	D3D12DepthStencilBufferDesc.SampleDesc.Count = MSAAState ? 4 : 1;
	D3D12DepthStencilBufferDesc.SampleDesc.Quality = MSAAState ? (MSAAQuality - 1) : 0;
	D3D12DepthStencilBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	D3D12DepthStencilBufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE D3D12ClearValue;
	D3D12ClearValue.Format = DXGIDepthStencilFormat;
	D3D12ClearValue.DepthStencil.Depth = 1.0f;
	D3D12ClearValue.DepthStencil.Stencil = 0;
	D3D12_HEAP_PROPERTIES D3D12HeapProperties;		// CD3DX12
	D3D12HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	D3D12HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	D3D12HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	D3D12HeapProperties.CreationNodeMask = 1;
	D3D12HeapProperties.VisibleNodeMask = 1;
	THROWIFFAILED(
		D3D12Device->CreateCommittedResource(
			&D3D12HeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&D3D12DepthStencilBufferDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&D3D12ClearValue,
			IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf())
		)
	);

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC D3D12DSVDesc;
	D3D12DSVDesc.Flags = D3D12_DSV_FLAG_NONE;
	D3D12DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	D3D12DSVDesc.Format = DXGIDepthStencilFormat;
	D3D12DSVDesc.Texture2D.MipSlice = 0;
	D3D12Device->CreateDepthStencilView(DepthStencilBuffer.Get(), &D3D12DSVDesc, GetDepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer.
	// CD3DX12
	D3D12_RESOURCE_BARRIER D3D12ResourceBarrier = CreateResoureceBarrierTransition(
		DepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	);

	D3D12CommandList->ResourceBarrier(1, &D3D12ResourceBarrier);

	ExecuteCommand();

	// Update the viewport transform to cover the client area.
	D3D12Viewport.TopLeftX = 0;
	D3D12Viewport.TopLeftY = 0;
	D3D12Viewport.Width = static_cast<float>(GetClientWidth());
	D3D12Viewport.Height = static_cast<float>(GetClientHeight());
	D3D12Viewport.MinDepth = 0.0f;
	D3D12Viewport.MaxDepth = 1.0f;

	D3D12ScissorRect = { 0, 0, static_cast<long>(GetClientWidth()), static_cast<long>(GetClientHeight()) };
}

void FRenderer::Update()
{
	float mPhi = XM_PIDIV4;
	float mRadius = 5.0f;
	float mTheta = 1.5f * XM_PI;
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);

	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f * XM_PI, GetAspectRatio(), 1.0f, 1000.0f);
	XMMATRIX worldViewProj = world * view * proj;

	// Update the constant buffer with the latest worldViewProj matrix.
	FWVPConstantBuffer objConstants;
	XMStoreFloat4x4(&objConstants.WorldViewProjectMatrix, XMMatrixTranspose(worldViewProj));
	WVPConstantBuffer->CopyData(0, objConstants);
}

void FRenderer::Render()
{
	StartRendering();

	D3D12CommandList->SetPipelineState(Pipelinestate.Get());

	ID3D12DescriptorHeap* descriptorHeaps[] = { D3D12CBVHeap.Get() };
	D3D12CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	D3D12CommandList->SetGraphicsRootSignature(RootSignature.Get());

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView = MeshGeometry->VertexBufferView();
	D3D12_INDEX_BUFFER_VIEW IndexBufferView = MeshGeometry->IndexBufferView();
	D3D12CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
	D3D12CommandList->IASetIndexBuffer(&IndexBufferView);
	D3D12CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D12CommandList->SetGraphicsRootDescriptorTable(0, D3D12CBVHeap->GetGPUDescriptorHandleForHeapStart());

	D3D12CommandList->DrawIndexedInstanced(
		MeshGeometry->DrawArgs["box"].IndexCount,
		1, 0, 0, 0);
	//////////////////////////////////////////////////////////////////////////
	FVertex TV[] =
	{
		FVertex({XMFLOAT3(-0.5f, 0, 0), XMFLOAT4(-0.5f, 0, 0, 1)}),
		FVertex({XMFLOAT3(0, 0.5f, 0), XMFLOAT4(0, 0.5f, 0, 1)}),
		FVertex({XMFLOAT3(0.5f, 0, 0), XMFLOAT4(0.5f, 0, 0, 1)})
	};
	UINT16 TI[] = { 0, 1, 2 };
	UINT VBSize = sizeof(FVertex) * 3;
	UINT IBSize = sizeof(UINT16) * 3;
	ComPtr<ID3DBlob> VBCPUBuffer;
	D3DCreateBlob(VBSize, &VBCPUBuffer);
	CopyMemory(VBCPUBuffer->GetBufferPointer(), TV, VBSize);
	ComPtr<ID3DBlob> IBCPUBuffer;
	D3DCreateBlob(IBSize, &IBCPUBuffer);
	CopyMemory(IBCPUBuffer->GetBufferPointer(), TI, IBSize);
	ComPtr<ID3D12Resource> VBUploader, IBUploader;
	ComPtr<ID3D12Resource> VBGPUBuffer = UDXUtility::CreateDefaultBuffer(
		D3D12Device.Get(),
		D3D12CommandList.Get(),
		TV,
		VBSize,
		VBUploader
	);
	ComPtr<ID3D12Resource> IBGPUBuffer = UDXUtility::CreateDefaultBuffer(
		D3D12Device.Get(),
		D3D12CommandList.Get(),
		TI,
		IBSize,
		IBUploader
	);
	D3D12_VERTEX_BUFFER_VIEW VBV;
	VBV.BufferLocation = VBGPUBuffer->GetGPUVirtualAddress();
	VBV.StrideInBytes = sizeof(FVertex);
	VBV.SizeInBytes = VBSize;
	D3D12_INDEX_BUFFER_VIEW IBV;
	IBV.BufferLocation = IBGPUBuffer->GetGPUVirtualAddress();
	IBV.Format = DXGI_FORMAT_R16_UINT;
	IBV.SizeInBytes = IBSize;
	D3D12CommandList->IASetVertexBuffers(0, 1, &VBV);
	D3D12CommandList->IASetIndexBuffer(&IBV);
	D3D12CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D12CommandList->SetGraphicsRootDescriptorTable(0, D3D12CBVHeap->GetGPUDescriptorHandleForHeapStart());

	D3D12CommandList->DrawIndexedInstanced(3, 1, 0, 0, 0);
	//////////////////////////////////////////////////////////////////////////

	
	// Indicate a state transition on the resource usage.
	ID3D12Resource* RenderTarget = GetCurrentBackBuffer();
	D3D12_RESOURCE_BARRIER ResourceBarrier = CreateResoureceBarrierTransition(
		RenderTarget,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	D3D12CommandList->ResourceBarrier(1, &ResourceBarrier);
	THROWIFFAILED(D3D12CommandList->Close());
	ID3D12CommandList* CommandList[] = { D3D12CommandList.Get() };
	D3D12CommandQueue->ExecuteCommandLists(_countof(CommandList), CommandList);
	
	THROWIFFAILED(DXGISwapChain->Present(0, 0));
	CurrBackBuffer = (CurrBackBuffer + 1) % SwapChainBufferCount;

	FlushCommandQueue();

	// swap the back and front buffers
	
	//EndRendering();
}

void FRenderer::StartRendering()
{
	InitializeCommand();
	
	// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
	D3D12CommandList->RSSetViewports(1, &D3D12Viewport);
	D3D12CommandList->RSSetScissorRects(1, &D3D12ScissorRect);

	// Indicate a state transition on the resource usage.
	ID3D12Resource* RenderTarget = GetCurrentBackBuffer();
	D3D12_RESOURCE_BARRIER ResourceBarrier = CreateResoureceBarrierTransition(
		RenderTarget,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	D3D12CommandList->ResourceBarrier(1, &ResourceBarrier);

	// Clear the back buffer and depth buffer.
	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = GetCurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView = GetDepthStencilView();
	D3D12CommandList->ClearRenderTargetView(RenderTargetView, DirectX::Colors::AliceBlue, 0, nullptr);
	D3D12CommandList->ClearDepthStencilView(DepthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	D3D12CommandList->OMSetRenderTargets(1, &RenderTargetView, true, &DepthStencilView);
}

void FRenderer::EndRendering()
{
	// Indicate a state transition on the resource usage.
	ID3D12Resource* RenderTarget = GetCurrentBackBuffer();
	D3D12_RESOURCE_BARRIER ResourceBarrier = CreateResoureceBarrierTransition(
		RenderTarget,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	D3D12CommandList->ResourceBarrier(1, &ResourceBarrier);

	ExecuteCommand();

	// swap the back and front buffers
	THROWIFFAILED(DXGISwapChain->Present(0, 0));
	CurrBackBuffer = (CurrBackBuffer + 1) % SwapChainBufferCount;

}

// CommandList에 명령을 작성하기 전에 실행. 호출후에는 반드시 ExecuteCommand()를 호출해 주어야 한다.
void FRenderer::InitializeCommand()
{
	THROWIFFAILED(D3D12CommandListAllocator->Reset());
	THROWIFFAILED(D3D12CommandList->Reset(D3D12CommandListAllocator.Get(), nullptr));
}

void FRenderer::ExecuteCommand()
{
	THROWIFFAILED(D3D12CommandList->Close());
	ID3D12CommandList* CommandList[] = { D3D12CommandList.Get() };
	D3D12CommandQueue->ExecuteCommandLists(_countof(CommandList), CommandList);
	FlushCommandQueue();
}

void FRenderer::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	CurrentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	THROWIFFAILED(D3D12CommandQueue->Signal(D3D12Fence.Get(), CurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (D3D12Fence->GetCompletedValue() < CurrentFence)
	{
		HANDLE EventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		THROWIFFAILED(D3D12Fence->SetEventOnCompletion(CurrentFence, EventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(EventHandle, INFINITE);
		CloseHandle(EventHandle);
	}
}

inline ID3D12Resource* FRenderer::GetCurrentBackBuffer()
{
	return SwapChainBuffer[CurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE FRenderer::GetCPUDescriptorHandle(
	ID3D12DescriptorHeap* DescriptorHeap,
	UINT DescriptorSize,
	UINT i
) const
{
	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle;
	DescriptorHandle.ptr = SIZE_T(
		INT64(DescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr) +
		INT64(i)*INT64(DescriptorSize)
	);
	return DescriptorHandle;
}

UINT FRenderer::GetRTVDescriptorSize() const
{
	static const UINT RTVDescriptorSize = D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	return RTVDescriptorSize;
}

UINT FRenderer::GetDSVDescriptorSize() const
{
	static const UINT DSVDescriptorSize = D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	return DSVDescriptorSize;
}

UINT FRenderer::GetCBVSRVUAVDescriptorSize() const
{
	static const UINT CBVSRVUAVDescriptorSize = D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	return CBVSRVUAVDescriptorSize;
}

void FRenderer::LogAdapters()
{
	UINT i = 0;
	IDXGIAdapter* Adapter = nullptr;
	std::vector<IDXGIAdapter*> AdapterList;
	while (DXGIFactory->EnumAdapters(i, &Adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC Desc;
		Adapter->GetDesc(&Desc);

		std::wstring Text = L"***Adapter: ";
		Text += Desc.Description;
		Text += L"\n";

		OutputDebugString(Text.c_str());

		AdapterList.push_back(Adapter);

		++i;
	}

	for (size_t i = 0; i < AdapterList.size(); ++i)
	{
		LogAdapterOutputs(AdapterList[i]);
		RELEASECOM(AdapterList[i]);
	}
}

void FRenderer::LogAdapterOutputs(IDXGIAdapter* Adapter)
{
	UINT i = 0;
	IDXGIOutput* Output = nullptr;
	while (Adapter->EnumOutputs(i, &Output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		Output->GetDesc(&desc);

		std::wstring Text = L"***Output: ";
		Text += desc.DeviceName;
		Text += L"\n";
		OutputDebugString(Text.c_str());

		LogOutputDisplayModes(Output, DXGIBackBufferFormat);

		RELEASECOM(Output);

		++i;
	}
}

void FRenderer::LogOutputDisplayModes(IDXGIOutput* Output, DXGI_FORMAT Format)
{
	UINT Count = 0;
	UINT Flags = 0;

	// Call with nullptr to get list count.
	Output->GetDisplayModeList(Format, Flags, &Count, nullptr);

	std::vector<DXGI_MODE_DESC> ModeList(Count);
	Output->GetDisplayModeList(Format, Flags, &Count, &ModeList[0]);

	for (auto& x : ModeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugString(text.c_str());
	}
}

void FRenderer::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	THROWIFFAILED(D3D12Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&D3D12CommandQueue)));

	THROWIFFAILED(
		D3D12Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(D3D12CommandListAllocator.GetAddressOf())
		)
	);

	THROWIFFAILED(
		D3D12Device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			D3D12CommandListAllocator.Get(), // Associated command allocator
			nullptr,                   // Initial PipelineStateObject
			IID_PPV_ARGS(D3D12CommandList.GetAddressOf())
		)
	);

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	D3D12CommandList->Close();
}

void FRenderer::CreateSwapChain()
{
	// Release the previous swapchain we will be recreating.
	DXGISwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferDesc.Width = GetClientWidth();
	SwapChainDesc.BufferDesc.Height = GetClientHeight();
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferDesc.Format = DXGIBackBufferFormat;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.SampleDesc.Count = MSAAState ? 4 : 1;
	SwapChainDesc.SampleDesc.Quality = MSAAState ? (MSAAQuality - 1) : 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = SwapChainBufferCount;
	SwapChainDesc.OutputWindow = GetHWnd();
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	THROWIFFAILED(
		DXGIFactory->CreateSwapChain(
			D3D12CommandQueue.Get(),
			&SwapChainDesc,
			DXGISwapChain.GetAddressOf()
		)
	);
}

void FRenderer::CreateDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc;
	RTVHeapDesc.NumDescriptors = SwapChainBufferCount;
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTVHeapDesc.NodeMask = 0;
	THROWIFFAILED(
		D3D12Device->CreateDescriptorHeap(
			&RTVHeapDesc,
			IID_PPV_ARGS(D3D12RTVHeap.GetAddressOf())
		)
	);

	D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc;
	DSVHeapDesc.NumDescriptors = 1;
	DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSVHeapDesc.NodeMask = 0;
	THROWIFFAILED(
		D3D12Device->CreateDescriptorHeap(
			&DSVHeapDesc,
			IID_PPV_ARGS(D3D12DSVHeap.GetAddressOf())
		)
	);

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	THROWIFFAILED(
		D3D12Device->CreateDescriptorHeap(
			&cbvHeapDesc,
			IID_PPV_ARGS(D3D12CBVHeap.GetAddressOf())
		)
	);
}

D3D12_RESOURCE_BARRIER FRenderer::CreateResoureceBarrierTransition(
	ID3D12Resource* Resource,
	D3D12_RESOURCE_STATES StateBefore,
	D3D12_RESOURCE_STATES StateAfter,
	UINT Subresource,
	D3D12_RESOURCE_BARRIER_FLAGS Flags
)
{
	D3D12_RESOURCE_BARRIER D3D12ResourceBarrierTransition;
	ZeroMemory(&D3D12ResourceBarrierTransition, sizeof(D3D12ResourceBarrierTransition));
	D3D12ResourceBarrierTransition.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	D3D12ResourceBarrierTransition.Flags = Flags;
	D3D12ResourceBarrierTransition.Transition.pResource = Resource;
	D3D12ResourceBarrierTransition.Transition.StateBefore = StateBefore;
	D3D12ResourceBarrierTransition.Transition.StateAfter = StateAfter;
	D3D12ResourceBarrierTransition.Transition.Subresource = Subresource;
	return D3D12ResourceBarrierTransition;
}

void FRenderer::BuildConstantBuffer()
{
	WVPConstantBuffer = std::make_unique<FUploadBuffer<FWVPConstantBuffer>>(D3D12Device.Get(), 1, true);

	D3D12_GPU_VIRTUAL_ADDRESS WVPConstantBufferAddress = WVPConstantBuffer->GetGPUVirtualAddress();
	// Offset to the ith object constant buffer in the buffer.
	int WVPConstantBufferIndex = 0;
	WVPConstantBufferAddress += WVPConstantBufferIndex * WVPConstantBuffer->GetElementByteSize();

	D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBufferViewDesc;
	ConstantBufferViewDesc.BufferLocation = WVPConstantBufferAddress;
	ConstantBufferViewDesc.SizeInBytes = WVPConstantBuffer->GetElementByteSize();

	D3D12Device->CreateConstantBufferView(
		&ConstantBufferViewDesc,
		D3D12CBVHeap->GetCPUDescriptorHandleForHeapStart()
	);
}

void FRenderer::BuildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// Root parameter can be a table, root descriptor or root constants.

	D3D12_ROOT_PARAMETER SlotRootParameter[1];

	// Create a single descriptor table of CBVs.
	D3D12_DESCRIPTOR_RANGE CBVTable;
	CBVTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	CBVTable.NumDescriptors = 1;
	CBVTable.BaseShaderRegister = 0;
	CBVTable.RegisterSpace = 0;
	CBVTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	SlotRootParameter[0].DescriptorTable.NumDescriptorRanges = 1;
	SlotRootParameter[0].DescriptorTable.pDescriptorRanges = &CBVTable;
	SlotRootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	SlotRootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// A root signature is an array of root parameters.
	D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc;
	RootSignatureDesc.NumParameters = 1;
	RootSignatureDesc.pParameters = SlotRootParameter;
	RootSignatureDesc.NumStaticSamplers = 0;
	RootSignatureDesc.pStaticSamplers = nullptr;
	RootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> SerializedRootSignature = nullptr;
	ComPtr<ID3DBlob> ErrorBlob = nullptr;
	HRESULT HResult = D3D12SerializeRootSignature(
		&RootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		SerializedRootSignature.GetAddressOf(),
		ErrorBlob.GetAddressOf()
	);

	if (ErrorBlob != nullptr)
	{
		::OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
	}
	THROWIFFAILED(HResult);
	
	THROWIFFAILED(
		D3D12Device->CreateRootSignature(
			0,
			SerializedRootSignature->GetBufferPointer(),
			SerializedRootSignature->GetBufferSize(),
			IID_PPV_ARGS(&RootSignature)
		)
	);
}

void FRenderer::BuildShaderAndLayout()
{
	HRESULT hr = S_OK;

	VSByteCode = CompileShader(L"Shaders\\UnlitVS.hlsl", nullptr, "VSMain", "vs_5_0");
	PSByteCode = CompileShader(L"Shaders\\UnlitPS.hlsl", nullptr, "PSMain", "ps_5_0");

	InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void FRenderer::BuildPipelineState()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineStateDesc;
	ZeroMemory(&PipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	PipelineStateDesc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
	PipelineStateDesc.pRootSignature = RootSignature.Get();
	PipelineStateDesc.VS =
	{
		reinterpret_cast<BYTE*>(VSByteCode->GetBufferPointer()),
		VSByteCode->GetBufferSize()
	};
	PipelineStateDesc.PS =
	{
		reinterpret_cast<BYTE*>(PSByteCode->GetBufferPointer()),
		PSByteCode->GetBufferSize()
	};
	PipelineStateDesc.RasterizerState = UDXUtility::CreateRasterizerDesc();
	PipelineStateDesc.BlendState = UDXUtility::CreateBlendDesc();
	PipelineStateDesc.DepthStencilState = UDXUtility::CreateDepthStencilDesc();
	PipelineStateDesc.SampleMask = UINT_MAX;
	PipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PipelineStateDesc.NumRenderTargets = 1;
	PipelineStateDesc.RTVFormats[0] = DXGIBackBufferFormat;
	PipelineStateDesc.SampleDesc.Count = MSAAState ? 4 : 1;
	PipelineStateDesc.SampleDesc.Quality = MSAAState ? (MSAAQuality- 1) : 0;
	PipelineStateDesc.DSVFormat = DXGIDepthStencilFormat;
	THROWIFFAILED(D3D12Device->CreateGraphicsPipelineState(&PipelineStateDesc, IID_PPV_ARGS(&Pipelinestate)));
}

ComPtr<ID3DBlob> FRenderer::CompileShader(
	const std::wstring& Filename,
	const D3D_SHADER_MACRO* Defines,
	const std::string& EntryPoint,
	const std::string& Target)
{
	UINT CompileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	CompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT HResult = S_OK;

	ComPtr<ID3DBlob> ByteCode = nullptr;
	ComPtr<ID3DBlob> Errors;
	HResult = D3DCompileFromFile(Filename.c_str(), Defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		EntryPoint.c_str(), Target.c_str(), CompileFlags, 0, &ByteCode, &Errors);

	if (Errors != nullptr)
		OutputDebugStringA((char*)Errors->GetBufferPointer());

	THROWIFFAILED(HResult);

	return ByteCode;
}
