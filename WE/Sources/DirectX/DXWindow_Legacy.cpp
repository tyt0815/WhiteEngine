#include "DXWindow_Legacy.h"
#include <windowsx.h>
#include "Render/ForwardRenderer.h"
#include "Render/MeshGeometry.h"
#include "Render/Material.h"
#include "Runtime/World/BoxWorld.h"
#include "Runtime/World/TestWorld.h"

LRESULT CALLBACK MainWndProc(HWND HWnd, UINT Message, WPARAM WParam, LPARAM LParam)
{
	return FDXWindow::GetInstance()->InternalWndProc(HWnd, Message, WParam, LParam);
}

FDXWindow::FDXWindow()
{
}

FDXWindow::~FDXWindow()
{
}

bool FDXWindow::Initialize()
{
	return InitializeWindow() && InitializeDirectX();
}

bool FDXWindow::InitializeWindow()
{
	WNDCLASS WndClass;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = MainWndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = GetHInstance();
	WndClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	WndClass.lpszMenuName = 0;
	WndClass.lpszClassName = DXWindowClassName;

	if (!RegisterClass(&WndClass))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT Rect = { 0, 0, DXWidth, DXHeight};
	AdjustWindowRect(&Rect, WS_OVERLAPPEDWINDOW, false);
	UINT Width = static_cast<UINT>(Rect.right - Rect.left);
	UINT Height = static_cast<UINT>(Rect.bottom - Rect.top);

	HWnd = CreateWindow(
		DXWindowClassName,
		DXWindowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		Width, Height,
		0, 0, GetHInstance(), 0
	);
	if (!HWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(HWnd, SW_SHOW);
	UpdateWindow(HWnd);

	return true;
}

bool FDXWindow::InitializeDirectX()
{
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> DebugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController)));
		DebugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&Factory)));

	// Try to create hardware device.
	HRESULT HResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&Device)
	);

	// Fallback to WARP device.
	if (FAILED(HResult))
	{
		ComPtr<IDXGIAdapter> WarpAdapter;
		ThrowIfFailed(Factory->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			WarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&Device)));
	}

	ThrowIfFailed(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&Fence)));

	RTVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DSVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	CBVSRVUAVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MSQualityLevels;
	MSQualityLevels.Format = BackBufferFormat;
	MSQualityLevels.SampleCount = 4;
	MSQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	MSQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(Device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&MSQualityLevels,
		sizeof(MSQualityLevels)));

	MSAAQuality_4x = MSQualityLevels.NumQualityLevels;
	assert(MSAAQuality_4x > 0 && "Unexpected MSAA quality level.");

	#ifdef _DEBUG
	LogAdapters();
	#endif

	CreateCommandObjects();
	CreateSwapChain();
	CreateDescriptorHeaps();
	ResetCameraPosition();
	Resize();
	return true;
}

void FDXWindow::ResetCameraPosition()
{
	Camera.Theta = 1.5f * XM_PI;
	Camera.Phi = XM_PIDIV4;
	Camera.Radius = MaxCameraRadius;
}

int FDXWindow::Run()
{
	MSG msg = { 0 };

	unique_ptr<WWorld> StartWorld = make_unique<WTestWorld>();
	StartWorld->Initialize();
	unique_ptr<FRendererBase> Renderer = make_unique<FForwardRenderer>(StartWorld.get());
	Renderer->Initialize();

	Timer.Reset();
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
			Timer.Tick();

			if (!bAppPaused)
			{
				CalculateFrameStats();
				ProcessInput();
				Renderer->Render();
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

LRESULT FDXWindow::InternalWndProc(HWND HWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			bAppPaused = true;
			Timer.Stop();
		}
		else
		{
			bAppPaused = false;
			Timer.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		if (Device)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				bAppPaused = true;
				bMinimized = true;
				bMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				bAppPaused = false;
				bMinimized = false;
				bMaximized = true;
				Resize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (bMinimized)
				{
					bAppPaused = false;
					bMinimized = false;
					Resize();
				}

				// Restoring from maximized state?
				else if (bMaximized)
				{
					bAppPaused = false;
					bMaximized = false;
					Resize();
				}
				else if (bResized)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					Resize();
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		bAppPaused = true;
		bResized = true;
		Timer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		bAppPaused = false;
		bResized = false;
		Timer.Start();
		Resize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_KEYDOWN:
		OnKeyDown(wParam);
		return 0;
	case WM_KEYUP:
		OnKeyUp(wParam);
		return 0;
	case WM_MOUSEWHEEL:
		OnMouseWheel(wParam);
		return 0;
	}


	return DefWindowProc(HWnd, msg, wParam, lParam);
}

RECT FDXWindow::GetClientRect()
{
	RECT R;
	::GetClientRect(HWnd, &R);
	return R;
}

wstring FDXWindow::GetWindowTitle()
{
	WCHAR Title[MaxTitle];
	GetWindowText(HWnd, Title, MaxTitle);
	return wstring(Title);
}

void FDXWindow::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	CurrentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(CommandQueue->Signal(Fence.Get(), CurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (Fence->GetCompletedValue() < CurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		ThrowIfFailed(Fence->SetEventOnCompletion(CurrentFence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void FDXWindow::ExecuteCommand()
{
	// Execute the initialization commands.
	ThrowIfFailed(CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();
}

void FDXWindow::SwapRenderTargetBuffers()
{
	ThrowIfFailed(SwapChain->Present(0, 0));
	CurrentBackBuffer = (CurrentBackBuffer + 1) % SwapChainBufferCount;
}

void FDXWindow::LogAdapters()
{
	UINT i = 0;
	IDXGIAdapter* Adapter = nullptr;
	std::vector<IDXGIAdapter*> AdapterList;
	while (Factory->EnumAdapters(i, &Adapter) != DXGI_ERROR_NOT_FOUND)
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
		ReleaseCom(AdapterList[i]);
	}
}

void FDXWindow::LogAdapterOutputs(IDXGIAdapter* Adapter)
{
	UINT i = 0;
	IDXGIOutput* Output = nullptr;
	while (Adapter->EnumOutputs(i, &Output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC Desc;
		Output->GetDesc(&Desc);

		std::wstring Text = L"***Output: ";
		Text += Desc.DeviceName;
		Text += L"\n";
		OutputDebugString(Text.c_str());

		LogOutputDisplayModes(Output, BackBufferFormat);

		ReleaseCom(Output);

		++i;
	}
}

void FDXWindow::LogOutputDisplayModes(IDXGIOutput* Output, DXGI_FORMAT Format)
{
	UINT Count = 0;
	UINT Flags = 0;

	// Call with nullptr to get list count.
	Output->GetDisplayModeList(Format, Flags, &Count, nullptr);

	std::vector<DXGI_MODE_DESC> ModeList(Count);
	Output->GetDisplayModeList(Format, Flags, &Count, &ModeList[0]);

	for (auto& Mode : ModeList)
	{
		UINT Numerator = Mode.RefreshRate.Numerator;
		UINT Denominator = Mode.RefreshRate.Denominator;
		std::wstring Text =
			L"Width = " + std::to_wstring(Mode.Width) + L" " +
			L"Height = " + std::to_wstring(Mode.Height) + L" " +
			L"Refresh = " + std::to_wstring(Numerator) + L"/" + std::to_wstring(Denominator) +
			L"\n";

		::OutputDebugString(Text.c_str());
	}
}

void FDXWindow::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((Timer.GetTotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		wstring fpsStr = to_wstring(fps);
		wstring mspfStr = to_wstring(mspf);

		wstring windowText = GetWindowTitle() +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr +
			L"TotalTime: " + std::to_wstring(Timer.GetTotalTime()) +
			L"ElapsedTime:" + std::to_wstring(timeElapsed);

		SetWindowText(HWnd, windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void FDXWindow::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {};
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(
		Device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&CommandQueue)));

	ThrowIfFailed(
		Device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(CommandAllocator.GetAddressOf())
		)
	);

	ThrowIfFailed(Device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		CommandAllocator.Get(), // Associated command allocator
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(CommandList.GetAddressOf())));

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	CommandList->Close();
}

void FDXWindow::CreateSwapChain()
{
	// Release the previous swapchain we will be recreating.
	SwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferDesc.Width = GetClientWidth();
	SwapChainDesc.BufferDesc.Height = GetClientHeight();
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferDesc.Format = BackBufferFormat;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.SampleDesc.Count = bMSAA ? 4 : 1;
	SwapChainDesc.SampleDesc.Quality = bMSAA ? (MSAAQuality_4x - 1) : 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = SwapChainBufferCount;
	SwapChainDesc.OutputWindow = HWnd;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(Factory->CreateSwapChain(
		CommandQueue.Get(),
		&SwapChainDesc,
		SwapChain.GetAddressOf()));
}

void FDXWindow::CreateDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc;
	RTVHeapDesc.NumDescriptors = SwapChainBufferCount;
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTVHeapDesc.NodeMask = 0;
	ThrowIfFailed(
		Device->CreateDescriptorHeap(
			&RTVHeapDesc,
			IID_PPV_ARGS(RTVHeap.GetAddressOf())
		)
	);


	D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc;
	DSVHeapDesc.NumDescriptors = 1;
	DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSVHeapDesc.NodeMask = 0;
	ThrowIfFailed(
		Device->CreateDescriptorHeap(
			&DSVHeapDesc,
			IID_PPV_ARGS(DSVHeap.GetAddressOf())
		)
	);
}

void FDXWindow::Resize()
{
	assert(Device);
	assert(SwapChain);
	assert(CommandAllocator);

	// Flush before changing any resources.
	FlushCommandQueue();

	ThrowIfFailed(CommandList->Reset(CommandAllocator.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < SwapChainBufferCount; ++i)
		SwapChainBuffer[i].Reset();
	DepthStencilBuffer.Reset();

	// Resize the swap chain.
	ThrowIfFailed(
		SwapChain->ResizeBuffers(
			SwapChainBufferCount,
			GetClientWidth(), GetClientHeight(),
			BackBufferFormat,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		)
	);

	CurrentBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffer[i])));
		Device->CreateRenderTargetView(SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, RTVDescriptorSize);
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = GetClientWidth();
	depthStencilDesc.Height = GetClientHeight();
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	depthStencilDesc.SampleDesc.Count = bMSAA ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = bMSAA ? (MSAAQuality_4x - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	CD3DX12_HEAP_PROPERTIES DepthStencilHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(
		Device->CreateCommittedResource(
			&DepthStencilHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf())
		)
	);

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	Device->CreateDepthStencilView(DepthStencilBuffer.Get(), &dsvDesc, GetDepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer.
	CD3DX12_RESOURCE_BARRIER ResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		DepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	);
	CommandList->ResourceBarrier(1, &ResourceBarrier);

	// Execute the resize commands.
	ThrowIfFailed(CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	ScreenViewport.TopLeftX = 0;
	ScreenViewport.TopLeftY = 0;
	ScreenViewport.Width = static_cast<float>(GetClientWidth());
	ScreenViewport.Height = static_cast<float>(GetClientHeight());
	ScreenViewport.MinDepth = 0.0f;
	ScreenViewport.MaxDepth = 1.0f;

	ScissorRect = { 0, 0, static_cast<LONG>(GetClientWidth()), static_cast<LONG>(GetClientHeight()) };

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * XM_PI, GetAspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&Camera.Project, P);
}

void FDXWindow::OnMouseDown(WPARAM WParam, int X, int Y)
{
	LastMousePos.x = X;
	LastMousePos.y = Y;

	if (WParam == MK_RBUTTON)
	{
		SetCapture(HWnd);
	}
}

void FDXWindow::OnMouseUp(WPARAM WParam, int X, int Y)
{
	//WParam
	ReleaseCapture();
	if (WParam == MK_RBUTTON)
	{
		ReleaseCapture();
	}
}

void FDXWindow::OnMouseMove(WPARAM WParam, int X, int Y)
{
	if (WParam == MK_RBUTTON)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(X - LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(Y - LastMousePos.y));

		// Update angles based on input to orbit camera around box.
		Camera.Theta -= dx;
		Camera.Phi -= dy;

		// Restrict the angle mPhi.
		Camera.Phi = UDXMath::Clamp(Camera.Phi, 0.1f, XM_PI - 0.1f);
	}
	LastMousePos.x = X;
	LastMousePos.y = Y;
}

void FDXWindow::OnKeyDown(WPARAM WParam)
{
}

void FDXWindow::OnKeyUp(WPARAM WParam)
{
	if (WParam == VK_ESCAPE)
	{
		PostQuitMessage(0);
	}
	else if ((int)WParam == VK_F2)
	{
		Set4xMsaaState(!bMSAA);
	}
}

void FDXWindow::ProcessInput()
{
	if (IsKeyPressed('R'))
	{
		ResetCameraPosition();
	}
}

void FDXWindow::OnMouseWheel(WPARAM WParam)
{
	int Delta = GET_WHEEL_DELTA_WPARAM(WParam);
	Camera.Radius -= (Delta / 100);
	Camera.Radius = UDXMath::Clamp<float>(Camera.Radius, MinCameraRadius, MaxCameraRadius);
}
