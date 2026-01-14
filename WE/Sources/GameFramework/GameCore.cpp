#include "GameCore.h"
#include "Window/Window.h"
#include "Utility/Timer.h"
#include "DirectX/DXResourceManager.h"
#include "Render/MeshGeometry.h"
#include "Render/Texture.h"
#include "Physics/PhysicsCore.h"
#include "Render/ShapeDrawer.h"
#include "Asset/AssetManager.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

HWND GetHWND();

namespace GameCore
{
	void CalculateFrameStats()
	{
		// Code computes the average frames per second, and also the 
		// average time it takes to render one frame.  These stats 
		// are appended to the window caption bar.

		static int frameCnt = 0;
		static float timeElapsed = 0.0f;

		frameCnt++;

		// Compute averages over one second period.
		if ((GetEngineTimer()->GetTotalTime() - timeElapsed) >= 1.0f)
		{
			float fps = (float)frameCnt; // fps = frameCnt / 1
			float mspf = 1000.0f / fps;

			std::wstring fpsStr = std::to_wstring(fps);
			std::wstring mspfStr = std::to_wstring(mspf);

			std::wstring windowText = GetMainWindowPtr()->GetWindowName() +
				L"    fps: " + fpsStr +
				L"   mspf: " + mspfStr +
				L"TotalTime: " + std::to_wstring(GetEngineTimer()->GetTotalTime()) +
				L"ElapsedTime:" + std::to_wstring(timeElapsed);

			SetWindowText(GetMainWindowPtr()->GetWindowHandle(), windowText.c_str());

			// Reset for next average.
			frameCnt = 0;
			timeElapsed += 1.0f;
		}
	}

	void InitializeApplication(IGameApp& Game)
	{
		// TODO: 현재 Commandlist를 통해 리소스 버퍼를 저장하는 몇몇 싱글톤 클래스를 명시적으로 
		// 호출해야 안정적인 실행이 가능함. 나중에 다중 스레드로 만들어서 해결해야할 필요 있음
		GetMeshGeometryManager();
		GetTextureManager();
		FShapeDrawer::GetInstance();

		Game.Startup();
	}

	void TerminateApplication(IGameApp& Game)
	{
		Game.Cleanup();

		GetDXResourceManagerPtr()->FlushCommandQueue();
	}

	void UpdateApplication(IGameApp& Game)
	{
		MSG msg = { 0 };

		UTimer* Timer = GetEngineTimer();
		Timer->Reset();

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
				Timer->Tick();
				if (!GetMainWindowPtr()->IsPaused())
				{
					CalculateFrameStats();

					GetInputSystemManager()->Tick();

					Game.Update(Timer->GetDeltaTime());
				}
				else
				{
					GetEngineTimer()->Stop();
					Sleep(100);
				}
			}
		}
	}
}

HINSTANCE g_hInst;

int GameCore::RunApplication(IGameApp& App, const wchar_t* ClassName, HINSTANCE hInst, int nCmdShow)
{
	g_hInst = hInst;

	InitializeApplication(App);

	UpdateApplication(App);

	TerminateApplication(App);

	return 0;
}
