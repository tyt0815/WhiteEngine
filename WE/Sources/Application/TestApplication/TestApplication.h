#pragma once

#include <memory>

#include "DirectX/DXResourceManager.h"
#include "GameFramework/InputSystem/InputSystem.h"
#include "World/TestWorld.h"
#include "Render/Renderer.h"
#include "Render/RenderItemManager.h"
#include "Render/Texture.h"
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

	WWorld* mWorld;

	bool bAppPaused = false;
};