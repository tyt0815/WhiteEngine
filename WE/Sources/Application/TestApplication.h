#pragma once

#include <memory>

#include "DirectX/DXDeviceManager.h"
#include "Render/Renderer.h"
#include "Render/Texture.h"
#include "GameFramework/Object/World/TestWorld.h"
#include "Utility/Class.h"
#include "Utility/Timer.h"
#include "Window/Window.h"

class FTestApplication
{
	SINGLETON(FTestApplication)
public:
	bool Initialize();
	int Run();

private:
	void CalculateFrameStats();
	void CreateRenderData(FRenderData& RenderData);
	std::unique_ptr<WWorld> mWorld;
	WViewCamera* Camera = nullptr;

	bool bAppPaused = false;
};