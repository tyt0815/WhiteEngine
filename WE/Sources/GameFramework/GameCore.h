#pragma once
#include <windows.h>
#include <thread>
#include <d3d12.h>
#include <thread>
#include "World/World.h"
#include "Render/DeferredShadingSceneRenderer.h"
#include "Utility/Class.h"

extern HINSTANCE g_hInst;

class FGameApplication final
{
	SINGLETON(FGameApplication);
public:
	
	void Initialize();

	template<typename TWorld>
	void CreateWorldAndRenderer();
	
	int Run();

private:
	void Thread_GamePlay();

	void Thread_Render();

	void Terminate();

	ID3D12Device* mDevice;

	TUniquePtr<WWorld> mWorld;
	TUniquePtr<FDeferredShadingSceneRenderer> mRenderer;

	bool mbPlaying = true;
};

#define CREATE_APPLICATION(WorldClass)\
	int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInst, PSTR CmdLine, int nCmdShow)\
	{\
		g_hInst = hInstance;\
		FGameApplication* App = FGameApplication::GetInstance();\
		App->Initialize();\
		App->CreateWorldAndRenderer<WorldClass>();\
		return App->Run();\
	}


template<typename TWorld>
inline void FGameApplication::CreateWorldAndRenderer()
{
	mWorld = MakeUnique<TWorld>();
	mRenderer = MakeUnique<FDeferredShadingSceneRenderer>();
	mRenderer->Initialize(mDevice);
}
