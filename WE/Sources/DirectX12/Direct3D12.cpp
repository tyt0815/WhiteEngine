#include "Direct3D12.h"

#include "Common/Common.h"
#include "Common/Container.h"
#include "Common/Timer.h"

FDirect3D12::FDirect3D12():
	mTimer(new UTimer())
{
	
}

FDirect3D12::~FDirect3D12()
{
}

bool FDirect3D12::Initialize(FWindow* TargetWindow)
{
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> DebugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController)));
		DebugController->EnableDebugLayer();
	}
#endif
	// Initialize Window
	Window = TargetWindow;

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFactory)));

	// Try to create hardware device.
	HRESULT HardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&D3D12Device));

	// Fallback to WARP device.
	if (FAILED(HardwareResult))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> WarpAdapter;
		ThrowIfFailed(DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter)));

		ThrowIfFailed(
			D3D12CreateDevice(
				WarpAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&D3D12Device)
			)
		);
	}

	ThrowIfFailed(D3D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&D3D12Fence)));

	RTVDescriptorSize = D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DSVDescriptorSize = D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CBVSRVUAVDescriptorSize = D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MSAAQualityLevels;
	MSAAQualityLevels.Format = DXGIBackBufferFormat;
	MSAAQualityLevels.SampleCount = 4;
	MSAAQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	MSAAQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(D3D12Device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&MSAAQualityLevels,
		sizeof(MSAAQualityLevels)));

	MSAAQuality = MSAAQualityLevels.NumQualityLevels;
	// MSAAQuality는 항상 0보다 커야한다.
	assert(MSAAQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
	LogAdapters();
#endif

	CreateCommandObjects();
	CreateSwapChain();
	CreateRTVAndDSVDescriptorHeaps();
	Resize();
    return false;
}

int FDirect3D12::Run()
{
	MSG msg = { 0 };

	mTimer->Reset();

	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			mTimer->Tick();

			if (!mAppPaused)
			{
				CalculateFrameStats();
				Update(mTimer);
				Draw(mTimer);
			}
		}
	}

	return (int)msg.wParam;
}

void FDirect3D12::Resize()
{
	assert(D3D12Device);
	assert(DXGISwapChain);
	assert(D3D12CommandListAllocator);

	// Flush before changing any resources.
	FlushCommandQueue();

	ThrowIfFailed(D3D12GraphicsCommandList->Reset(D3D12CommandListAllocator.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < SwapChainBufferCount; ++i)
	{
		D3D12SwapChainBuffer[i].Reset();
	}
	D3D12DepthStencilBuffer.Reset();

	// Resize the swap chain.
	ThrowIfFailed(DXGISwapChain->ResizeBuffers(
		SwapChainBufferCount,
		ClientWidth, ClientHeight,
		DXGIBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	CurrBackBuffer = 0;

	// CD3DX12
	D3D12_CPU_DESCRIPTOR_HANDLE D3D12RTVHeapHandle = D3D12RTVHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(DXGISwapChain->GetBuffer(i, IID_PPV_ARGS(&D3D12SwapChainBuffer[i])));
		D3D12Device->CreateRenderTargetView(D3D12SwapChainBuffer[i].Get(), nullptr, D3D12RTVHeapHandle);
		D3D12RTVHeapHandle.ptr = size_t(INT64(D3D12RTVHeapHandle.ptr) + INT64(1) * INT64(RTVDescriptorSize));
		/*
		D3D12RTVHeapHandle.Offset(1, RTVDescriptorSize);
		* 
		CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(INT offsetInDescriptors, UINT descriptorIncrementSize) noexcept
		{
			ptr = SIZE_T(INT64(ptr) + INT64(offsetInDescriptors) * INT64(descriptorIncrementSize));
			return *this;
		}
		*/
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC D3D12DepthStencilDesc;
	D3D12DepthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	D3D12DepthStencilDesc.Alignment = 0;
	D3D12DepthStencilDesc.Width = ClientWidth;
	D3D12DepthStencilDesc.Height = ClientHeight;
	D3D12DepthStencilDesc.DepthOrArraySize = 1;
	D3D12DepthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	D3D12DepthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	D3D12DepthStencilDesc.SampleDesc.Count = MSAAState ? 4 : 1;
	D3D12DepthStencilDesc.SampleDesc.Quality = MSAAState ? (MSAAQuality - 1) : 0;
	D3D12DepthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	D3D12DepthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

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
	ThrowIfFailed(
		D3D12Device->CreateCommittedResource(
			&D3D12HeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&D3D12DepthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&D3D12ClearValue,
			IID_PPV_ARGS(D3D12DepthStencilBuffer.GetAddressOf())
		)
	);

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC D3D12DSVDesc;
	D3D12DSVDesc.Flags = D3D12_DSV_FLAG_NONE;
	D3D12DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	D3D12DSVDesc.Format = DXGIDepthStencilFormat;
	D3D12DSVDesc.Texture2D.MipSlice = 0;
	D3D12Device->CreateDepthStencilView(D3D12DepthStencilBuffer.Get(), &D3D12DSVDesc, GetDepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer.
	// CD3DX12
	D3D12_RESOURCE_BARRIER D3D12ResourceBarrier = CreateResoureceBarrierTransition(
		D3D12DepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
		);

	D3D12GraphicsCommandList->ResourceBarrier(1, &D3D12ResourceBarrier);

	// Execute the resize commands.
	ThrowIfFailed(D3D12GraphicsCommandList->Close());
	ID3D12CommandList* D3D12CommandList[] = { D3D12GraphicsCommandList.Get() };
	D3D12CommandQueue->ExecuteCommandLists(_countof(D3D12CommandList), D3D12CommandList);

	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	D3D12Viewport.TopLeftX = 0;
	D3D12Viewport.TopLeftY = 0;
	D3D12Viewport.Width = static_cast<float>(ClientWidth);
	D3D12Viewport.Height = static_cast<float>(ClientHeight);
	D3D12Viewport.MinDepth = 0.0f;
	D3D12Viewport.MaxDepth = 1.0f;

	D3D12ScissorRect = { 0, 0, ClientWidth, ClientHeight };
}

void FDirect3D12::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	CurrentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(D3D12CommandQueue->Signal(D3D12Fence.Get(), CurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (D3D12Fence->GetCompletedValue() < CurrentFence)
	{
		HANDLE EventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		ThrowIfFailed(D3D12Fence->SetEventOnCompletion(CurrentFence, EventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(EventHandle, INFINITE);
		CloseHandle(EventHandle);
	}
}

ID3D12Resource* FDirect3D12::CurrentBackBuffer()
{
	return D3D12SwapChainBuffer[CurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE FDirect3D12::GetCurrentBackBufferView() const
{
	/*return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		D3D12RTVHeap->GetCPUDescriptorHandleForHeapStart(),
		CurrBackBuffer,
		RTVDescriptorSize
	);*/
	// CD3DX12
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView;
	CurrentBackBufferView.ptr = SIZE_T(
		INT64(D3D12RTVHeap->GetCPUDescriptorHandleForHeapStart().ptr) + INT64(CurrBackBuffer) * INT64(RTVDescriptorSize)
	);
	return CurrentBackBufferView;
}

D3D12_CPU_DESCRIPTOR_HANDLE FDirect3D12::GetDepthStencilView() const
{
	return D3D12DSVHeap->GetCPUDescriptorHandleForHeapStart();
}

void FDirect3D12::LogAdapters()
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

void FDirect3D12::LogAdapterOutputs(IDXGIAdapter* Adapter)
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

void FDirect3D12::LogOutputDisplayModes(IDXGIOutput* Output, DXGI_FORMAT Format)
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

void FDirect3D12::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(D3D12Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&D3D12CommandQueue)));

	ThrowIfFailed(
		D3D12Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(D3D12CommandListAllocator.GetAddressOf())
		)
	);

	ThrowIfFailed(
		D3D12Device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			D3D12CommandListAllocator.Get(), // Associated command allocator
			nullptr,                   // Initial PipelineStateObject
			IID_PPV_ARGS(D3D12GraphicsCommandList.GetAddressOf())
		)
	);

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	D3D12GraphicsCommandList->Close();
}

void FDirect3D12::CreateSwapChain()
{
	// Release the previous swapchain we will be recreating.
	DXGISwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferDesc.Width = ClientWidth;
	SwapChainDesc.BufferDesc.Height = ClientHeight;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferDesc.Format = DXGIBackBufferFormat;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.SampleDesc.Count = MSAAState ? 4 : 1;
	SwapChainDesc.SampleDesc.Quality = MSAAState ? (MSAAQuality - 1) : 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = SwapChainBufferCount;
	if (Window)
	{
		SwapChainDesc.OutputWindow = Window->GetHWnd();
	}
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(
		DXGIFactory->CreateSwapChain(
		D3D12CommandQueue.Get(),
		&SwapChainDesc,
		DXGISwapChain.GetAddressOf()
		)
	);
}

void FDirect3D12::CreateRTVAndDSVDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc;
	RTVHeapDesc.NumDescriptors = SwapChainBufferCount;
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTVHeapDesc.NodeMask = 0;
	ThrowIfFailed(D3D12Device->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(D3D12RTVHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc;
	DSVHeapDesc.NumDescriptors = 1;
	DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSVHeapDesc.NodeMask = 0;
	ThrowIfFailed(D3D12Device->CreateDescriptorHeap(&DSVHeapDesc, IID_PPV_ARGS(D3D12DSVHeap.GetAddressOf())));
}

void FDirect3D12::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static double timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((mTimer->GetTotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);

		std::wstring windowText = Window->GetTitle() +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr +
			L"TotalTime: " + std::to_wstring(mTimer->GetTotalTime()) +
			L"ElapsedTime:" + std::to_wstring(timeElapsed);


		SetWindowText(Window->GetHWnd(), windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void FDirect3D12::Update(UTimer* Timer)
{
}

void FDirect3D12::Draw(UTimer* gt)
{
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(D3D12CommandListAllocator->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(D3D12GraphicsCommandList->Reset(D3D12CommandListAllocator.Get(), nullptr));

	// Indicate a state transition on the resource usage.
	ID3D12Resource* RenderTarget = CurrentBackBuffer();
	D3D12_RESOURCE_BARRIER ResourceBarrier = CreateResoureceBarrierTransition(
		RenderTarget,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	D3D12GraphicsCommandList->ResourceBarrier(1, &ResourceBarrier);

	// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
	D3D12GraphicsCommandList->RSSetViewports(1, &D3D12Viewport);
	D3D12GraphicsCommandList->RSSetScissorRects(1, &D3D12ScissorRect);

	// Clear the back buffer and depth buffer.
	D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView = GetCurrentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView = GetDepthStencilView();
	D3D12GraphicsCommandList->ClearRenderTargetView(RenderTargetView, DirectX::Colors::LightSteelBlue, 0, nullptr);

	D3D12GraphicsCommandList->ClearDepthStencilView(DepthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	D3D12GraphicsCommandList->OMSetRenderTargets(1, &RenderTargetView, true, &DepthStencilView);

	// Indicate a state transition on the resource usage.
	ResourceBarrier = CreateResoureceBarrierTransition(
		RenderTarget,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	D3D12GraphicsCommandList->ResourceBarrier(1, &ResourceBarrier);

	// Done recording commands.
	ThrowIfFailed(D3D12GraphicsCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { D3D12GraphicsCommandList.Get() };
	D3D12CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(DXGISwapChain->Present(0, 0));
	CurrBackBuffer = (CurrBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();
}

inline D3D12_RESOURCE_BARRIER FDirect3D12::CreateResoureceBarrierTransition(
	ID3D12Resource* Resource,
	D3D12_RESOURCE_STATES StateBefore,
	D3D12_RESOURCE_STATES StateAfter,
	UINT Subresource,
	D3D12_RESOURCE_BARRIER_FLAGS Flags
)
{
	D3D12_RESOURCE_BARRIER D3D12ResourceBarrierTransition;
	D3D12ResourceBarrierTransition.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	D3D12ResourceBarrierTransition.Flags = Flags;
	D3D12ResourceBarrierTransition.Transition.pResource = Resource;
	D3D12ResourceBarrierTransition.Transition.StateBefore = StateBefore;
	D3D12ResourceBarrierTransition.Transition.StateAfter = StateAfter;
	D3D12ResourceBarrierTransition.Transition.Subresource = Subresource;
	return D3D12ResourceBarrierTransition;
}