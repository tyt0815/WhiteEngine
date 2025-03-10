#include "TestApplication.h"

FTestApplication::FTestApplication()
{

}

FTestApplication::~FTestApplication()
{

}

bool FTestApplication::Initialize()
{
	mWorld = std::make_unique<WTestWorld>();

	return true;
}

int FTestApplication::Run()
{
	MSG msg = { 0 };
	FDXResourceManager* DeviceManager = FDXResourceManager::GetInstance();
	ID3D12Device* Device = DeviceManager->GetDevicePtr();
	ID3D12GraphicsCommandList* CommandList = DeviceManager->GetCommandListPtr();
	ID3D12CommandAllocator* CommandAllocator = DeviceManager->GetCommandAllocatorPtr();
	ID3D12CommandQueue* CommandQueue = DeviceManager->GetCommandQueuePtr();
	THROW_IF_FAILED(CommandList->Reset(CommandAllocator, nullptr));
	FMeshGeometry::BuildMeshGeometries(Device, CommandList);
	FTexture::LoadTexture(Device, CommandList);
	THROW_IF_FAILED(CommandList->Close());
	ID3D12CommandList* CmdLists[] = { CommandList };
	CommandQueue->ExecuteCommandLists(_countof(CmdLists), CmdLists);
	DeviceManager->FlushCommandQueue();

	mWorld->Initialize();
	Camera = mWorld->GetCamera();
	Camera->UpdateProjMatrix(0.25f * XM_PI, DeviceManager->GetAspectRatio(), 1.0f, 1000.0f);
	FRenderer Renderer;
	DeviceManager->Camera = Camera;
	Renderer.Initialize();
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
			GetAppTimer()->Tick();
			if (!GetMainWindowPtr()->IsPaused())
			{
				GetAppTimer()->Start();
				CalculateFrameStats();
				// TODO
				//ProcessInput();
				mWorld->Tick(GetAppTimer()->GetDeltaTime());
				GetFrameResourceManager()->Tick();
				FRenderData RenderData;
				CreateRenderData(RenderData);
				Renderer.Render(RenderData);
				DeviceManager->PresentAndSwapBuffer();
			}
			else
			{
				GetAppTimer()->Stop();
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

void FTestApplication::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((GetAppTimer()->GetTotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);

		std::wstring windowText = GetMainWindowPtr()->GetWindowName() +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr +
			L"TotalTime: " + std::to_wstring(GetAppTimer()->GetTotalTime()) +
			L"ElapsedTime:" + std::to_wstring(timeElapsed);

		SetWindowText(GetMainWindowPtr()->GetWindowHandle(), windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void FTestApplication::CreateRenderData(FRenderData& RenderData)
{
	RenderData.FrameResource = GetFrameResourceManager()->GetTargetFrameResource();
}
