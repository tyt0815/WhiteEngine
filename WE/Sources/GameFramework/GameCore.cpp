#include "GameCore.h"
#include "DirectX/DXResourceManager.h"
#include "Render/MeshGeometry.h"
#include "Render/Texture.h"
#include "Physics/PhysicsCore.h"
#include "Render/ShapeDrawer.h"
#include "Asset/AssetManager.h"
#include "Window/Window.h"

#include "Utility/Timer.h"
#include "GUI/GUICore.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

HINSTANCE g_hInst;

std::atomic<int> g_TickRate{ 0 };
std::atomic<int> g_FPS{ 0 };
std::atomic<float> g_GameThreadDeltaMs{ 0.0f };
std::atomic<float> g_RenderThreadDeltaMs{ 0.0f };

inline constexpr int g_TickRateLimit = 300;
inline constexpr int g_FPSLimit = 300;
inline constexpr float g_TickTimelimit = 1.0f / g_TickRateLimit;
inline constexpr float g_FrameTimelimit = 1.0f / g_FPSLimit;

inline constexpr bool g_bMultiThread = true;

inline constexpr bool g_bLimitTick = false;
inline constexpr bool g_bLimitFrame = false;

namespace Physics
{
	bool g_bDrawShape = true;
	bool g_bDrawBoundingBox = false;
}



FGameApplication::FGameApplication()
{

}

FGameApplication::~FGameApplication()
{

}
#include <iostream>
void FGameApplication::Initialize()
{
	if (AllocConsole())
	{
		// 2. stdout(출력), stderr(에러), stdin(입력)을 새 콘솔 창(CONOUT$, CONIN$)으로 연결
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
		freopen_s(&fp, "CONIN$", "r", stdin);

		// 3. iostream 동기화 (cout이 정상 작동하도록 함)
		std::ios::sync_with_stdio();

		// 3. [중요] 와이드 문자(wcout)를 위한 로케일 및 모드 설정
		// 이 부분이 없으면 wcout에 wstring을 넣어도 출력되지 않습니다.
		_wsetlocale(LC_ALL, L"korean");
	}

	timeBeginPeriod(1); // 타이머 해상도를 1ms로 설정

	// 프로그램 종료 시
	
	Physics::Startup();
	FMeshGeometryManager::GetInstance();
	FAssetManager::GetInstance()->LoadAssets();
	FTextureManager::GetInstance();
	FShapeDrawer::GetInstance();
	FDXResourceManager* DXRM = GetDXResourceManagerPtr();

	mDevice = DXRM->GetDevicePtr();

	SetWindowText(GetMainWindowPtr()->GetWindowHandle(), L"WE");
}

int FGameApplication::Run()
{
	if (g_bMultiThread)
	{
		// 프로파일링용 GUI
		GUI::FDrawCommand Command;
		Command.LifeSpan = -1;
		Command.DrawLambda = [&]()
		{
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS: %d", g_FPS.load());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "Tick Rate: %d", g_TickRate.load());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "RenderThread: %.3fms", g_RenderThreadDeltaMs.load());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "GameThread: %.3fms", g_GameThreadDeltaMs.load());
		};
		GUI::AddProfilingCommand(Command);

		std::thread GameplayThread(&FGameApplication::Thread_GamePlay, this);
		Thread_Render();
		GameplayThread.join();
	}
	else
	{
		GUI::FDrawCommand Command;
		Command.LifeSpan = -1;
		Command.DrawLambda = [&]()
		{
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS: %d", g_FPS.load());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "mspf: %.3fms", g_RenderThreadDeltaMs.load());
		};
		GUI::AddProfilingCommand(Command);

		MSG msg = { 0 };

		int FrameCount = 0;
		float TimeElapsed = 0.0f;
		FTimer Timer;

		mWorld->BeginPlay();

		while (msg.message != WM_QUIT)
		{
			Timer.Tick();
			float DeltaTime = (float)Timer.GetDeltaSecond();

			// --- 틱레이트 제한 로직 시작 ---
			if (g_bLimitFrame && DeltaTime < g_FrameTimelimit)
			{
				float RemainingTime = g_FrameTimelimit - DeltaTime;
				if (RemainingTime > 0.002f)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds((int)((RemainingTime - 0.0015f) * 1000.0f)));
				}

				while (DeltaTime < g_FrameTimelimit)
				{
					Timer.Tick();
					DeltaTime += (float)Timer.GetDeltaSecond();

					// CPU 점유율을 너무 높이지 않도록 힌트만 줌
					std::this_thread::yield();
				}
			}
			// --- 틱레이트 제한 로직 끝 ---


			TimeElapsed += DeltaTime;
			++FrameCount;
			if (TimeElapsed >= 1)
			{
				g_FPS = (int)(FrameCount / TimeElapsed);
				g_RenderThreadDeltaMs = TimeElapsed / (float)FrameCount * 1000.0f;
				FrameCount = 0;
				TimeElapsed = 0.0f;
			}

			GetInputSystemManager()->Tick(DeltaTime);
			mWorld->Tick(DeltaTime);


			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				mRenderer->Tick(mWorld->mRenderItemProxy);
			}
		}
	}
	Terminate();
	return 0;
}

void FGameApplication::Thread_GamePlay()
{
	int FrameCount = 0;
	float TimeElapsed = 0.0f;
	FTimer Timer;

	mWorld->BeginPlay();

	while (mbPlaying)
	{
		Timer.Tick();
		float DeltaTime = (float)Timer.GetDeltaSecond();

		if (g_bLimitTick && DeltaTime < g_TickTimelimit)
		{
			float RemainingTime = g_TickTimelimit - DeltaTime;
			if (RemainingTime > 0.002f)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds((int)((RemainingTime - 0.0015f) * 1000.0f)));
			}

			while (DeltaTime < g_TickTimelimit)
			{
				Timer.Tick();
				DeltaTime += (float)Timer.GetDeltaSecond();

				 std::this_thread::yield();
			}
		}

		TimeElapsed += DeltaTime;
		++FrameCount;
		if (TimeElapsed >= 1)
		{
			g_TickRate = (int)(FrameCount / TimeElapsed);
			g_GameThreadDeltaMs = TimeElapsed / (float)FrameCount * 1000.0f;
			FrameCount = 0;
			TimeElapsed = 0.0f;
		}

		GetInputSystemManager()->Tick(DeltaTime);
		mWorld->Tick(DeltaTime);
	}
}

void FGameApplication::Thread_Render()
{
	MSG msg = { 0 };

	int FrameCount = 0;
	float TimeElapsed = 0.0f;
	FTimer Timer;

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Timer.Tick();
			float DeltaTime = (float)Timer.GetDeltaSecond();

			// --- 틱레이트 제한 로직 시작 ---
			if (g_bLimitFrame && DeltaTime < g_FrameTimelimit)
			{
				float RemainingTime = g_FrameTimelimit - DeltaTime;
				if (RemainingTime > 0.002f)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds((int)((RemainingTime - 0.0015f) * 1000.0f)));
				}

				while (DeltaTime < g_FrameTimelimit)
				{
					Timer.Tick();
					DeltaTime += (float)Timer.GetDeltaSecond();

					// CPU 점유율을 너무 높이지 않도록 힌트만 줌
					std::this_thread::yield();
				}
			}
			// --- 틱레이트 제한 로직 끝 ---


			TimeElapsed += DeltaTime;
			++FrameCount;
			if (TimeElapsed >= 1)
			{
				g_FPS = (int)(FrameCount / TimeElapsed);
				g_RenderThreadDeltaMs = TimeElapsed / (float)FrameCount * 1000.0f;
				FrameCount = 0;
				TimeElapsed = 0.0f;
			}

			mWorld->mRenderItemProxyMetex.lock();
			FRenderItemProxy RIP = mWorld->mRenderItemProxy;
			mWorld->mRenderItemProxyMetex.unlock();
			mRenderer->Tick(RIP);
		}
	}

	mbPlaying = false;
}

void FGameApplication::Terminate()
{
	GetDXResourceManagerPtr()->FlushCommandQueue();
	mRenderer->Destroy();
	mWorld.reset();
	mRenderer.reset();

	timeEndPeriod(1);
}
