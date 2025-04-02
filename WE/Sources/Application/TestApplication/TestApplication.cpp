#include "TestApplication.h"

FTestApplication::FTestApplication()
{

}

FTestApplication::~FTestApplication()
{

}

bool FTestApplication::Initialize()
{
	mWorld = GetWObjectManager()->CreateWObject<WTestWorld>();

	return true;
}

int FTestApplication::Run()
{
	MSG msg = { 0 };
	FDXResourceManager* DXManager = FDXResourceManager::GetInstance();

	FForwardShadingSceneRenderer Renderer;
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
				GetInputSystemManager()->Tick();
				GetWObjectManager()->Tick(GetAppTimer()->GetDeltaTime());
				Renderer.Render();
				DXManager->PresentAndSwapBuffer();
			}
			else
			{
				GetAppTimer()->Stop();
				Sleep(100);
			}
		}
	}
	Renderer.Destroy();
	// GPU 의 명령이 끝나고 Com객체들을 해제해야 정상적인 해제 가능
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