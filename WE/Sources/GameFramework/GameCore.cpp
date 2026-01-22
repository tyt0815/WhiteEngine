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

HINSTANCE g_hInst;

std::atomic<float> g_GameFPS{ 0.0f };
std::atomic<float> g_RenderFPS{ 0.0f };

FGameApplication::FGameApplication()
{

}

FGameApplication::~FGameApplication()
{

}

void FGameApplication::Initialize()
{
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

	// 프로파일링용 GUI
	GUI::FDrawCommand Command;
	Command.LifeSpan = -1;
	Command.DrawLambda = [&]()
	{
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "RenderFPS: %.8f", g_RenderFPS.load()); // 노란색 제목
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "GameFPS: %.8f", g_GameFPS.load()); // 노란색 제목
	};
	GUI::AddProfilingCommand(Command);

	std::thread GameplayThread(&FGameApplication::Thread_GamePlay, this);
	Thread_Render();
	GameplayThread.join();
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
		float DeltaTime = (float)Timer.GetDeltaTime();

		TimeElapsed += DeltaTime;
		++FrameCount;
		if (TimeElapsed >= 1)
		{
			g_GameFPS = FrameCount / TimeElapsed;
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


			TimeElapsed += (float)Timer.GetDeltaTime();
			++FrameCount;
			if (TimeElapsed >= 1)
			{
				g_RenderFPS = FrameCount / TimeElapsed;
				FrameCount = 0;
				TimeElapsed = 0.0f;
			}

			mWorld->mRenderItemProxyMetex.lock();
			FRenderItemProxy RIP = mWorld->mRenderItemProxy;
			mWorld->mRenderItemProxyMetex.unlock();
			mRenderer->Tick(RIP);
			
			double RenderDelta = Timer.GetDeltaTime();

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
}
