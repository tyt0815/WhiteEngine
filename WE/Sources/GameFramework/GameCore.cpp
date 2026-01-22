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

	std::thread GameplayThread(&FGameApplication::Thread_GamePlay, this);
	Thread_Render();
	GameplayThread.join();
	Terminate();
	return 0;
}

void FGameApplication::Thread_GamePlay()
{
	UTimer Timer;

	while (mbPlaying)
	{
		Timer.Tick();
		float DeltaTime = (float)Timer.GetDeltaTime();
		mWorld->Tick(DeltaTime);
	}
}

void FGameApplication::Thread_Render()
{
	MSG msg = { 0 };

	UTimer Timer;

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			GetInputSystemManager()->Tick();

			Timer.Tick();

			mWorld->mRenderItemProxyMetex.lock();
			FRenderItemProxy RIP = mWorld->mRenderItemProxy;
			mWorld->mRenderItemProxyMetex.unlock();
			mRenderer->Tick(RIP);
			
			double RenderDelta = Timer.GetDeltaTime();
			// 프로파일링용 GUI
			GUI::FDrawCommand Command;
			Command.LifeSpan = 0;
			Command.DrawLambda = [=]()
			{
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Rendering"); // 노란색 제목
				ImGui::Separator();
				ImGui::Text("Rendering: %.8f", RenderDelta);
			};
			GUI::AddProfilingCommand(Command);
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
